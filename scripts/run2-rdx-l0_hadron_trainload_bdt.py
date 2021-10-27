#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Wed Oct 27, 2021 at 03:13 AM +0200
# Based on the script 'regmva.py' shared by Patrick Owen

import pickle
import numpy as np

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True  # Don't hijack argparse!
ROOT.PyConfig.DisableRootLogon = True  # Don't read .rootlogon.py

from argparse import ArgumentParser
from time import perf_counter
from contextlib import contextmanager

from ROOT import gInterpreter, RDataFrame
from sklearn.ensemble import AdaBoostRegressor
from sklearn.tree import DecisionTreeRegressor

from TrackerOnlyEmu.executor import ExecDirective as EXEC
from TrackerOnlyEmu.executor import process_directives
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
    'd0_et_emu_no_bdt',
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

def gen_output_dict(arr, names):
    return dict(zip(names, arr.T))


def slice_bdt_input(arr, right_idx=len(BDT_TRAIN_BRANCHES)):
    return arr[:, 0:right_idx]


@contextmanager
def Timer():
    start = perf_counter()
    yield lambda: perf_counter() - start


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
        EXEC('Define', 'k_trg_hcal_et', 'k_L0Calo_HCAL_TriggerHCALET', True),
        EXEC('Define', 'pi_real_et', 'pi_L0Calo_HCAL_realET', True),
        EXEC('Define', 'pi_trg_et', 'pi_L0Calo_HCAL_TriggerET', True),
        EXEC('Define', 'pi_trg_hcal_et', 'pi_L0Calo_HCAL_TriggerHCALET', True),
    ]

    if args.debug:
        directives += directives_debug
        ADD_BRANCHES += ADD_BRANCHES_DEBUG

    init_frame = RDataFrame(args.tree, args.input)
    dfs, output_br_names = process_directives(directives, init_frame)

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
    debug_output = gen_output_dict(input_vars, BDT_TRAIN_BRANCHES+ADD_BRANCHES)
    debug_output['d0_et_diff'] = regression_var
    debug_output['d0_et_diff_pred'] = bdt.predict(slice_bdt_input(input_vars))

    debug_rdf = ROOT.RDF.MakeNumpyDataFrame(debug_output)
    final_debug_df = debug_rdf.Define(
        'd0_et_trg_pred_diff', 'd0_et_diff - d0_et_diff_pred')

    final_debug_df.Snapshot(args.tree, args.output)
