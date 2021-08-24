#!/usr/bin/env python3
#
# Authors: Yipeng Sun, Manuel Franco Sevilla

import pickle
import time
import numpy as np
from argparse import ArgumentParser

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True  # Don't hijack argparse!
ROOT.PyConfig.DisableRootLogon = True  # Don't read .rootlogon.py
from ROOT import gInterpreter, RDataFrame

from TrackerOnlyEmu.loader import load_file, load_cpp
from TrackerOnlyEmu.executor import ExecDirective as EXEC
from TrackerOnlyEmu.executor import process_directives

import xgboost as xgb

#################
# Configurables #
#################

XGB_TRAIN_BRANCHES = [
    'nTracks',  ## To model the NumSPDHits < 450 cut
    'd0_P',
    'd0_PT',
    'k_P',
    'k_PT',
    'k_TRUEPT',
    'k_L0Calo_HCAL_realET', 
    'k_L0Calo_HCAL_xProjection',
    'k_L0Calo_HCAL_yProjection',
    'k_L0Calo_HCAL_region',
    'pi_P',
    'pi_PT',
    'pi_TRUEPT',
    'pi_L0Calo_HCAL_realET',
    'pi_L0Calo_HCAL_xProjection',
    'pi_L0Calo_HCAL_yProjection',
    'pi_L0Calo_HCAL_region',
    'spi_P',
    'spi_PT',
    'spi_TRUEPT',
    'spi_L0Calo_HCAL_realET',
    'spi_L0Calo_HCAL_xProjection',
    'spi_L0Calo_HCAL_yProjection',
    'spi_L0Calo_HCAL_region',
]

#################################
# Command line arguments parser #
#################################

def parse_input():
    parser = ArgumentParser(
        description='Train a regression xgb for a better emulation of L0Hadron trigger.')

    parser.add_argument('input', help='''
specify input ntuple file.''')

    parser.add_argument('-o','--output', default='gen/xgb.pickle', help='''
optionally specify output pickle dump.''')

    parser.add_argument('-p','--particle', default='d0', help='''
optionally specify whether to train on d0, k, or pi trigger.''')

    parser.add_argument('-t', '--tree', default='TupleB0/DecayTree', help='''
optionally specify tree name.''')

    parser.add_argument('--load-xgb', default=None, help='''
optionally specify xgb to load.''')

    parser.add_argument('--max-depth', default=4, type=int, help='''
optionally specify the max_depth parameter for the xgb.''')

    parser.add_argument('--ntrees', default=300, type=int, help='''
optionally specify the n_estimators parameter for the xgb.''')

    parser.add_argument('-s', '--silent',
                        action='store_true',
                        help='do no print output')


    return parser.parse_args()


###########
# Helpers #
###########

load_cpp('<triggers/l0/run2-L0Hadron.h>')

def slice_xgb_input(arr, right_idx=len(XGB_TRAIN_BRANCHES)):
    return arr[:, 0:right_idx]


#############
# Train xgb #
#############

if __name__ == '__main__':
    args = parse_input()
    parti = args.particle

    init_frame = RDataFrame(args.tree, args.input)

    input_vars = np.array(list(init_frame.AsNumpy(columns=XGB_TRAIN_BRANCHES).values())).T
    regression_var = np.array(list(init_frame.AsNumpy(columns=[parti+'_L0HadronDecision_TOS']).values())).astype(int).T.ravel()

    
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
