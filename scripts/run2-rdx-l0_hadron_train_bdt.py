#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Tue May 18, 2021 at 09:51 PM +0200
# Based on the script 'regmva.py' shared by Patrick Owen

import pickle
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
    'k_L0Calo_HCAL_realET',  # NOTE: realET is tracker ET!
    'pi_L0Calo_HCAL_realET',
    'rdiff_k_pi'
]

REGRESSION_BRANCHES = [
    EXEC('Define', 'd0_et_real',
         'capHcalResp(k_L0Calo_HCAL_realET, pi_L0Calo_HCAL_realET)', True),
    EXEC('Define', 'd0_et_diff', 'd0_et_real - d0_et_emu_no_bdt', True),
]


#################################
# Command line arguments parser #
#################################

def parse_input():
    parser = ArgumentParser(
        description='Train a regression BDT for a better emulation of L0Hadron trigger.')

    parser.add_argument('input', help='''
specify input ntuple file.
''')

    parser.add_argument('output', help='''
specify output pickle dump.
''')

    parser.add_argument('-t', '--tree', default='TupleB0/DecayTree', help='''
specify tree name.
''')

    parser.add_argument('--debug', action='store_true', help='''
enable debug mode.
''')

    parser.add_argument('--debug-ntuple', default='debug.root', help='''
specify debug ntuple output location.
''')

    parser.add_argument('--load-bdt', default=None, help='''
optionally specify serialized BDT to load.''')

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

    dfs, output_br_names = process_directives(REGRESSION_BRANCHES, init_frame)
    regression_var = dfs[-1].AsNumpy(columns=['d0_et_diff'])

    bdt_input_vars_dev, _, regression_var_dev, _ = train_test_split(
        bdt_input_vars, regression_var['d0_et_diff'],
        test_size=0.2, random_state=42)
    bdt_input_vars_train, _, regression_var_train, _ = train_test_split(
        bdt_input_vars_dev, regression_var_dev, test_size=0.2, random_state=492)

    if not args.load_bdt:
        print('Start training for a regression BDT...')
        rng = np.random.RandomState(1)
        bdt = AdaBoostRegressor(DecisionTreeRegressor(max_depth=4),
                                n_estimators=300, random_state=rng)
        bdt.fit(bdt_input_vars_train, regression_var_train)
        print('BDT fitted.')

        pickle.dump(bdt, open(args.output, 'wb'))

    else:
        print('Load already serialized BDT...')
        bdt = pickle.load(open(args.load_bdt, 'rb'))

    if args.debug:
        print('Generate debug ntuple...')
        debug_output = dfs[-1].AsNumpy()

        d0_et_diff_pred = bdt.predict(bdt_input_vars)
        debug_output['d0_et_diff_pred'] = d0_et_diff_pred

        debug_rdf = ROOT.RDF.MakeNumpyDataFrame(debug_output)
        final_df = debug_rdf.Define(
            'et_pred_real_diff', 'd0_et_diff_pred - d0_et_diff')

        output_br_names.push_back('d0_et_diff_pred')
        output_br_names.push_back('et_pred_real_diff')

        final_df.Snapshot(args.tree, args.debug_ntuple, output_br_names)
