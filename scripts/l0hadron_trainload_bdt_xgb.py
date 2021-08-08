#!/usr/bin/env python3
#
# Author: Yipeng Sun, Manuel Franco Sevilla
# Last Change: Sat Jul 10, 2021 at 01:39 AM +0200
# Based on the script 'regmva.py' shared by Patrick Owen

import pickle
import time
import numpy as np
import sys

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True  # Don't hijack argparse!
ROOT.PyConfig.DisableRootLogon = True  # Don't read .rootlogon.py

from argparse import ArgumentParser

from ROOT import gInterpreter, RDataFrame
from sklearn.model_selection import train_test_split
from sklearn.ensemble import AdaBoostRegressor
from sklearn.tree import DecisionTreeRegressor

import xgboost as xgb

from TrackerOnlyEmu.loader import load_file, load_cpp
from TrackerOnlyEmu.executor import ExecDirective as EXEC
from TrackerOnlyEmu.executor import process_directives


#################
# Configurables #
#################

TRAIN_BRANCHES = [
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

    parser.add_argument('-o','--output', default='gen/bdt.pickle', help='''
optionally specify output pickle dump.''')

    parser.add_argument('-c','--classifier', default='bdt', help='''
specify classifier: "bdt" or "xgb".''')

    parser.add_argument('-t', '--tree', default='TupleB0/DecayTree', help='''
optionally specify tree name.''')

    parser.add_argument('--load-cl', default=None, help='''
optionally specify classifier to load.''')

    parser.add_argument('--max-depth', default=4, type=int, help='''
optionally specify the max_depth parameter for the BDT.''')

    parser.add_argument('--ntrees', default=300, type=int, help='''
optionally specify the n_estimators parameter for the BDT.''')

    parser.add_argument('-s', '--silent',
                        action='store_true',
                        help='do no print output')


    return parser.parse_args()


###########
# Helpers #
###########

load_cpp('<triggers/l0/run2-L0Hadron.h>')

def slice_input(arr, right_idx=len(TRAIN_BRANCHES)):
    return arr[:, 0:right_idx]


######################
# Train/Load BDT/XGB #
######################

if __name__ == '__main__':
    args = parse_input()

    init_frame = RDataFrame(args.tree, args.input)

    input_vars = np.array(list(init_frame.AsNumpy(
            columns=TRAIN_BRANCHES).values())).T

    dfs, output_br_names = process_directives(REGRESSION_BRANCHES, init_frame)
    regression_var = dfs[-1].AsNumpy(columns=['d0_et_diff'])

    if not args.load_cl:
        if not args.silent: print('Start training a regression '+args.classifier+'...')
        cl_input_vars = slice_input(input_vars)

        ## Doing BDT/XGB fit
        time_start = time.perf_counter()
        if args.classifier == 'bdt':
            rng = np.random.RandomState(1)
            classif = AdaBoostRegressor(DecisionTreeRegressor(max_depth=args.max_depth),
                                    n_estimators=args.ntrees, random_state=rng)
        elif args.classifier == 'xgb':
            classif = xgb.XGBRegressor(n_estimators=args.ntrees, max_depth=args.max_depth)
        else: sys.exit('Classifier "'+args.classifier+'" is not allowed')
        classif.fit(cl_input_vars, regression_var['d0_et_diff'])
        time_stop = time.perf_counter()
        if not args.silent: print(args.classifier+'BDT fit took a total of {:,.2f} sec'.format(time_stop - time_start))

        ## Saving classifier
        if args.output != 'None':
            if not args.silent: print('Export trained '+args.classifier+' to '+args.output)
            with open(args.output, 'wb') as f:
                pickle.dump(classif, f)

    else:
        ## Variables to add to new tree with BDT/XGB output
        idx_names = ['runNumber', 'eventNumber', 'd0_et_emu_no_bdt']
        idx_vars = np.array(
            list(init_frame.AsNumpy(columns=idx_names).values())).T
        output_tree = dict(zip(idx_names,idx_vars.T))

        ## Loading BDT/XGB and calculating related variables
        if not args.silent: print('Loading classifier from '+args.load_cl)
        classif = pickle.load(open(load_file(args.load_cl), 'rb'))
        d0_et_corr = classif.predict(input_vars)
        output_tree['d0_et_corr'] = d0_et_corr
        output_tree['d0_trg_et'] = dfs[-1].AsNumpy(columns=['d0_trg_et'])['d0_trg_et']
        classif_rdf = ROOT.RDF.MakeNumpyDataFrame(output_tree)
        d0_et_emu_frame = classif_rdf.Define('d0_et_emu', 'capHcalResp(d0_et_emu_no_bdt + d0_et_corr)')
        l0hadron_trg_frame = d0_et_emu_frame.Define('d0_l0_hadron_tos_emu',
        'l0HadronTriggerEmu(d0_et_emu, 2016)')

        ## Saving tree
        branchKeep = ['runNumber', 'eventNumber','d0_l0_hadron_tos_emu', 'd0_et_corr', 'd0_et_emu','d0_trg_et']
        l0hadron_trg_frame.Snapshot(args.tree, args.output, branchKeep)
        if not args.silent: print("Written "+args.output)
