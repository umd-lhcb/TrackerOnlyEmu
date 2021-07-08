#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Thu Jul 08, 2021 at 08:36 PM +0200
# Based on the script 'regmva.py' shared by Patrick Owen

import pickle
import time
import numpy as np

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True  # Don't hijack argparse!
ROOT.PyConfig.DisableRootLogon = True  # Don't read .rootlogon.py

from argparse import ArgumentParser

from ROOT import gInterpreter, RDataFrame
from sklearn.model_selection import train_test_split
from sklearn.ensemble import AdaBoostRegressor
from sklearn.tree import DecisionTreeRegressor

from TrackerOnlyEmu.loader import load_cpp
from TrackerOnlyEmu.executor import ExecDirective as EXEC
from TrackerOnlyEmu.executor import process_directives


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

REGRESSION_BRANCHES = [
    EXEC('Define', 'd0_trg_et',
         'capHcalResp(k_L0Calo_HCAL_TriggerET, pi_L0Calo_HCAL_TriggerET)', True),
    EXEC('Define', 'd0_et_diff', 'd0_trg_et - d0_et_emu_no_bdt', True),
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
specify output pickle dump.''')

    parser.add_argument('-t', '--tree', default='TupleB0/DecayTree', help='''
specify tree name.''')

    parser.add_argument('--debug-ntuple', default=None, help='''
specify debug ntuple output location.''')

    parser.add_argument('--load-bdt', default=None, help='''
optionally specify serialized BDT to load.''')

    parser.add_argument('--max-depth', default=4, type=int, help='''
optionally specify the max_depth parameter for the BDT.''')

    parser.add_argument('--test-ntuple', default=None, help='''
specify test ntuple output location.''')

    return parser.parse_args()


########################
# Load C++ definitions #
########################

load_cpp('<triggers/l0/run2-L0Hadron.h>')


#############
# Train BDT #
#############

if __name__ == '__main__':
    args = parse_input()

    init_frame = RDataFrame(args.tree, args.input)

    bdt_input_vars = np.array(
        list(init_frame.AsNumpy(columns=BDT_TRAIN_BRANCHES).values())).T
    idx_vars = np.array(
        list(init_frame.AsNumpy(columns=['runNumber', 'eventNumber']))).T

    dfs, output_br_names = process_directives(REGRESSION_BRANCHES, init_frame)
    regression_var = dfs[-1].AsNumpy(columns=['d0_et_diff'])

    bdt_input_vars_dev, _, regression_var_dev, _ = train_test_split(
        bdt_input_vars, regression_var['d0_et_diff'],
        test_size=0.2, random_state=42)
    bdt_input_vars_train, bdt_input_vars_test, \
        regression_var_train, regression_var_test = train_test_split(
            bdt_input_vars_dev, regression_var_dev, test_size=0.2,
            random_state=492)

    if not args.load_bdt:
        print('Start training for a regression BDT...')
        time_start = time.perf_counter()
        rng = np.random.RandomState(1)
        bdt = AdaBoostRegressor(DecisionTreeRegressor(max_depth=args.max_depth),
                                n_estimators=300, random_state=rng)
        bdt.fit(bdt_input_vars_train, regression_var_train)
        time_stop = time.perf_counter()
        print('BDT fitted. It takes a total of {:,.2f} sec'.format(
            time_stop - time_start))

        if args.output != 'None':
            print('Export trained BDT...')
            pickle.dump(bdt, open(args.output, 'wb'))

    else:
        print('Load already serialized BDT...')
        bdt = pickle.load(open(args.load_bdt, 'rb'))

    if args.debug_ntuple is not None:
        print('Generate debug ntuple...')
        debug_output = dfs[-1].AsNumpy()

        d0_et_diff_pred = bdt.predict(bdt_input_vars)
        debug_output['d0_et_diff_pred'] = d0_et_diff_pred

        # Delete the d0_L0HadronDecision_TOS branch as we can't convert from a
        # numpy bool array to a TTree one directly.
        del(debug_output['d0_L0HadronDecision_TOS'])

        debug_rdf = ROOT.RDF.MakeNumpyDataFrame(debug_output)
        final_df = debug_rdf.Define(
            'd0_et_trg_pred_diff', 'd0_et_diff - d0_et_diff_pred')

        output_br_names.push_back('d0_et_diff_pred')
        output_br_names.push_back('d0_et_trg_pred_diff')
        output_br_names.push_back('d0_et_emu_no_bdt')

        final_df.Snapshot(args.tree, args.debug_ntuple, output_br_names)

    if args.test_ntuple is not None:
        print('Generate test ntuple...')
        test_output = dict(zip(BDT_TRAIN_BRANCHES, bdt_input_vars_test.T))
        test_output['d0_et_diff'] = regression_var_test
        test_output['d0_et_diff_pred'] = bdt.predict(bdt_input_vars_test)

        test_rdf = ROOT.RDF.MakeNumpyDataFrame(test_output)
        test_rdf.Snapshot(args.tree, args.test_ntuple)
