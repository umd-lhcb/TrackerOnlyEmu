#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Mon May 17, 2021 at 03:40 PM +0200
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

from TrackerOnlyEmu.executor import ExecDirective as EXEC
from TrackerOnlyEmu.executor import process_directives


#################
# Configurables #
#################

BDT_TRAIN_BRANCHES = [
    'd0_PT',
    'd0_P',
    'k_L0Calo_HCAL_realET',
    'pi_L0Calo_HCAL_realET',
    'rdiff_k_pi'
]

REGRESSION_BRANCHES = [
    'd0_et_emu_no_bdt'
]


###########
# Helpers #
###########

def generate_exe(branches, keep=True):
    return [EXEC('Define', br, br, keep) for br in branches]


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
specify output joblib dump.
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

    return parser.parse_args()


########################
# Load C++ definitions #
########################

gInterpreter.Declare('#include <TMath.h>')


#############
# Train BDT #
#############

if __name__ == '__main__':
    args = parse_input()
    init_frame = RDataFrame(args.tree, args.input)

    directives_bdt_input = generate_exe(BDT_TRAIN_BRANCHES)
    print(directives_bdt_input)
    dfs, _ = process_directives(directives_bdt_input, init_frame)
    bdt_input_vars = dfs[-1].AsNumpy()

    directives_regression = generate_exe(REGRESSION_BRANCHES, False)
    directives_regression.append(EXEC(
        'Define', 'd0_et_real',
        'TMath::Max(k_L0Calo_HCAL_realET, pi_L0Calo_HCAL_realET)', False))
    directives_regression.append(EXEC(
        'Define', 'd0_et_diff', 'd0_et_real - d0_et_emu_no_bdt', True))
    final_dfs, output_br_names = process_directives(
        directives_regression, dfs[-1])
    regression_var = final_dfs[-1].AsNumpy(columns=['d0_et_diff'])

    bdt_input_vars_dev, _, regression_var_dev, _ = train_test_split(
        bdt_input_vars, regression_var, test_size=0.2, random_state=42)
    bdt_input_vars_train, _, regression_var_train, _ = train_test_split(
        bdt_input_vars_dev, regression_var_dev, test_size=0.2, random_state=492)

    rng = np.random.RandomState(1)
    bdt = AdaBoostRegressor(DecisionTreeRegressor(max_depth=4),
                            n_estimators=300, random_state=rng)
    bdt.fit(bdt_input_vars_train, regression_var_train)
    print('BDT fitted.')

    pickle.dump(bdt, open(args.output))

    if args.debug:
        d0_et_diff_pred = bdt.predict(bdt_input_vars)
        regression_var['d0_et_diff_pred'] = d0_et_diff_pred

        debug_rdf = ROOT.RDF.MakeNumpyDataFrame(regression_var)
        debug_rdf.Define('et_diff', 'd0_et_diff_pred - d0_et_diff')

        output_br_names.push_back('d0_et_diff_pred')
        output_br_names.push_back('et_diff')

        debug_rdf.Snapshot(args.tree, args.debug_ntuple, output_br_names)
