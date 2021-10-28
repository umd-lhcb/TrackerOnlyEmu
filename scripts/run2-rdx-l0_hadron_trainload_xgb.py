#!/usr/bin/env python3
#
# Authors: Yipeng Sun, Manuel Franco Sevilla

import pickle
import numpy as np
import xgboost as xgb

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True  # Don't hijack argparse!
ROOT.PyConfig.DisableRootLogon = True  # Don't read .rootlogon.py

from argparse import ArgumentParser
from ROOT import gInterpreter, RDataFrame

from TrackerOnlyEmu.loader import load_file
from TrackerOnlyEmu.executor import ExecDirective as EXEC
from TrackerOnlyEmu.executor import process_directives
from TrackerOnlyEmu.utils import slice_array
from TrackerOnlyEmu.emulation.run2_rdx import XGB_TRAIN_BRANCHES


#################
# Configurables #
#################

ADD_BRANCHES = [
    'runNumber',
    'eventNumber',
]

ADD_BRANCHES_DEBUG = [
    'd0_l0_hadron_tos',
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
        description='Train a regression XGB for a better emulation of L0Hadron trigger.')

    parser.add_argument('input', help='''
specify input ntuple file.''')

    parser.add_argument('output', help='''
specify output ntuple file.''')

    parser.add_argument('-p', '--particle', default='d0', help='''
optionally specify whether to train on D0, K, or Pi trigger.''')

    parser.add_argument('-t', '--tree', default='TupleB0/DecayTree', help='''
optionally specify tree name.''')

    parser.add_argument('-s', '--silent', action='store_true', help='''
do no print output.''')

    parser.add_argument('--debug', action='store_true', help='''
enable debug mode.
''')

    parser.add_argument('--load-xgb', default=None, help='''
optionally specify XGB to load.''')

    parser.add_argument('--dump-xgb', default=None, help='''
optionally specify output to pickled XGB object.''')

    parser.add_argument('--max-depth', default=4, type=int, help='''
optionally specify the max_depth parameter for the XGB.''')

    parser.add_argument('--ntrees', default=300, type=int, help='''
optionally specify the n_estimators parameter for the XGB.''')

    return parser.parse_args()


###########
# Helpers #
###########

slice_xgb_input = lambda x: slice_array(x, right_idx=len(XGB_TRAIN_BRANCHES))


#############
# Train xgb #
#############

if __name__ == '__main__':
    args = parse_input()
    parti = args.particle

    directives = []
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
    ]

    if args.debug:
        directives += directives_debug
        ADD_BRANCHES += ADD_BRANCHES_DEBUG

    init_frame = RDataFrame(args.tree, args.input)
    if directives:
        dfs, _ = process_directives(directives, init_frame)
    else:
        dfs = [init_frame]

    input_vars = np.array(
        list(dfs[-1].AsNumpy(columns=XGB_TRAIN_BRANCHES).values())).T
    regression_var = dfs[-1].AsNumpy(
        columns=[parti+'_L0HadronDecision_TOS'])[parti+'_L0HadronDecision_TOS']

    if not args.load_xgb:
        if not args.silent: print('Start training a classification XGBoost with '+str(args.ntrees)
        +' trees and max-depth = '+str(args.max_depth))
        time_start = time.perf_counter()
        xgb_r = xgb.XGBClassifier(n_estimators=args.ntrees, max_depth=args.max_depth,
        use_label_encoder =False,eval_metric='mlogloss',reg_lambda=0.5)

        xgb_input = slice_xgb_input(input_vars)
        xgb_r.fit(xgb_input, regression_var)
        time_stop = time.perf_counter()
        if not args.silent: print('XGB fit took a total of {:,.2f} sec'.format(time_stop - time_start))

        if args.output != 'None':
            if not args.silent: print('Export trained XGBoost to '+args.output)
            with open(args.output, 'wb') as f:
                pickle.dump(xgb_r, f)

    else:
        # Variables to add to new tree with XGB output
        idx_names = ['runNumber', 'eventNumber', 'd0_et_emu_no_bdt']
        idx_vars = np.array(
            list(init_frame.AsNumpy(columns=idx_names).values())).T
        output_tree = dict(zip(idx_names,idx_vars.T))

        # Loading XGB and calculating related variables
        if not args.silent: print('Loading XGBoost from '+args.load_xgb)
        xgb_r = pickle.load(open(load_file(args.load_xgb), 'rb'))
        l0_hadron_tos_emu = xgb_r.predict(input_vars)
        l0_hadron_prob = xgb_r.predict_proba(input_vars).T[1]
        output_tree[parti+'_l0_hadron_tos_emu'] = l0_hadron_tos_emu
        output_tree[parti+'_l0_hadron_prob'] = l0_hadron_prob
        xgb_rdf = ROOT.RDF.MakeNumpyDataFrame(output_tree)

        # Saving tree
        branchKeep = ['runNumber', 'eventNumber',parti+'_l0_hadron_tos_emu',parti+'_l0_hadron_prob']
        xgb_rdf.Snapshot(args.tree, args.output, branchKeep)
        if not args.silent: print("Written "+args.output)
