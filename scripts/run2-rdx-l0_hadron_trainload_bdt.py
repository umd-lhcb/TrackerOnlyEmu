#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Thu Oct 28, 2021 at 03:25 AM +0200
# Based on the script 'regmva.py' shared by Patrick Owen

import pickle
import numpy as np

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True  # Don't hijack argparse!
ROOT.PyConfig.DisableRootLogon = True  # Don't read .rootlogon.py

from argparse import ArgumentParser
from ROOT import gInterpreter, RDataFrame
from sklearn.ensemble import AdaBoostRegressor
from sklearn.tree import DecisionTreeRegressor

from TrackerOnlyEmu.executor import ExecDirective as EXEC
from TrackerOnlyEmu.executor import process_directives
from TrackerOnlyEmu.utils import Timer
from TrackerOnlyEmu.utils import gen_output_dict, slice_array
from TrackerOnlyEmu.emulation.run2_rdx import \
    run2_rdx_l0_hadron_tos_no_bdt_directive_gen


#################
# Configurables #
#################

BDT_TRAIN_BRANCHES = [
    'd0_PT',
    'd0_P',
    'k_L0Calo_HCAL_realET',  # NOTE: realET is not Trigger ET!
    'pi_L0Calo_HCAL_realET',
    'rdiff_k_pi'
]

# Keep these branches in output ntuples
ADD_BRANCHES = [
    'runNumber',
    'eventNumber',
    'd0_et_diff',
    'd0_et_emu_no_bdt',
]

ADD_BRANCHES_DEBUG = [
    'd0_l0_hadron_tos',
    'd0_l0_hadron_tos_emu_no_bdt',
    'k_pt',
    'pi_pt',
    'd0_pt',
    'k_p',
    'pi_p',
    'd0_p',
]


#################################
# Command line arguments parser #
#################################

def parse_input():
    parser = ArgumentParser(
        description='Train a regression BDT for a better emulation of L0Hadron trigger.')

    parser.add_argument('input', help='''
specify input ntuple file.''')

    parser.add_argument('output', help='''
specify output ntuple file.''')

    parser.add_argument('-t', '--tree', default='TupleB0/DecayTree', help='''
specify tree name.''')

    parser.add_argument('-y', '--year', default='2016', help='''
specify year.''')

    parser.add_argument('--debug', action='store_true', help='''
enable debug mode.
''')

    parser.add_argument('--load-bdt', default=None, help='''
optionally specify serialized BDT to load.''')

    parser.add_argument('--dump-bdt', default=None, help='''
optionally specify output to pickled BDT object.''')

    parser.add_argument('--max-depth', default=4, type=int, help='''
optionally specify the max_depth parameter for the BDT.''')

    return parser.parse_args()

###########
# Helpers #
###########

slice_bdt_input = lambda x: slice_array(x, right_idx=len(BDT_TRAIN_BRANCHES))


#############
# Train BDT #
#############

if __name__ == '__main__':
    args = parse_input()

    directives = run2_rdx_l0_hadron_tos_no_bdt_directive_gen(args.year)

    directives_debug = [
        # Reference variables
        EXEC('Define', 'd0_l0_hadron_tos',
             'static_cast<Int_t>(d0_L0HadronDecision_TOS)', True),

        # Fit variables
        EXEC('Define', 'q2', 'FitVar_q2 / 1e6', True),
        EXEC('Define', 'mmiss2', 'FitVar_Mmiss2 / 1e6', True),
        EXEC('Define', 'el', 'FitVar_El / 1e3', True),

        # Kinematic variables
        EXEC('Define', 'd0_pt', 'd0_PT / 1e3', True),
        EXEC('Define', 'k_pt', 'k_PT / 1e3', True),
        EXEC('Define', 'pi_pt', 'pi_PT / 1e3', True),
        EXEC('Define', 'd0_p', 'd0_P / 1e3', True),
        EXEC('Define', 'k_p', 'k_P / 1e3', True),
        EXEC('Define', 'pi_p', 'pi_P / 1e3', True),

        # Cut variable candidates
        EXEC('Define', 'k_real_et', 'k_L0Calo_HCAL_realET', True),
        EXEC('Define', 'k_trg_et', 'k_L0Calo_HCAL_TriggerET', True),
        EXEC('Define', 'pi_real_et', 'pi_L0Calo_HCAL_realET', True),
        EXEC('Define', 'pi_trg_et', 'pi_L0Calo_HCAL_TriggerET', True),
    ]

    if args.debug:
        directives += directives_debug
        ADD_BRANCHES += ADD_BRANCHES_DEBUG

    init_frame = RDataFrame(args.tree, args.input)
    dfs, _ = process_directives(directives, init_frame)

    input_vars = np.array(
        list(dfs[-1].AsNumpy(
            columns=BDT_TRAIN_BRANCHES+ADD_BRANCHES).values())).T
    regression_var = dfs[-1].AsNumpy(columns=['d0_et_diff'])['d0_et_diff']

    if not args.load_bdt:
        print('Start training for a regression BDT...')
        rng = np.random.RandomState(1)
        bdt = AdaBoostRegressor(DecisionTreeRegressor(max_depth=args.max_depth),
                                n_estimators=300, random_state=rng)
        bdt_input = slice_bdt_input(input_vars)

        with Timer() as t:
            bdt.fit(bdt_input, regression_var)
        print('BDT fitted. It takes a total of {:,.2f} sec'.format(t()))

        if args.dump_bdt:
            print('Export trained BDT to {}...'.format(args.dump_bdt))
            with open(args.dump_bdt, 'wb') as f:
                pickle.dump(bdt, f)

    else:
        print('Load already serialized BDT...')
        bdt = pickle.load(open(args.load_bdt, 'rb'))

    # Output the ntuple
    print('Generate output ntuple...')
    output = gen_output_dict(input_vars, BDT_TRAIN_BRANCHES+ADD_BRANCHES)
    output['d0_et_diff_pred'] = bdt.predict(slice_bdt_input(input_vars))
    output_df = ROOT.RDF.MakeNumpyDataFrame(output)

    directives_output = [
        EXEC('Define', 'd0_et_emu', 'capHcalResp(d0_et_emu_no_bdt + d0_et_diff_pred)',
             True),
        EXEC('Define', 'd0_et_trg_pred_diff', 'd0_et_diff - d0_et_diff_pred',
             True),
        EXEC('Define', 'd0_l0_hadron_tos_emu_bdt',
             'static_cast<Double_t>(l0HadronTriggerEmu(d0_et_emu, {}))'.format(args.year),
             True),
    ]

    out_dfs, _ = process_directives(directives_output, output_df)
    out_dfs[-1].Snapshot(args.tree, args.output)
