#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Sat Jul 10, 2021 at 01:39 AM +0200
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

from TrackerOnlyEmu.loader import load_file, load_cpp
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

    parser.add_argument('-o','--output', default='gen/bdt.pickle', help='''
optionally specify output pickle dump.''')

    parser.add_argument('-t', '--tree', default='TupleB0/DecayTree', help='''
optionally specify tree name.''')

    parser.add_argument('--load-bdt', default=None, help='''
optionally specify BDT to load.''')

    parser.add_argument('--max-depth', default=4, type=int, help='''
optionally specify the max_depth parameter for the BDT.''')

    parser.add_argument('--ntrees', default=300, type=int, help='''
optionally specify the n_estimators parameter for the BDT.''')


    return parser.parse_args()


###########
# Helpers #
###########

load_cpp('<triggers/l0/run2-L0Hadron.h>')

def slice_bdt_input(arr, right_idx=len(BDT_TRAIN_BRANCHES)):
    return arr[:, 0:right_idx]


#############
# Train BDT #
#############

if __name__ == '__main__':
    args = parse_input()

    init_frame = RDataFrame(args.tree, args.input)

    input_vars = np.array(list(init_frame.AsNumpy(
            columns=BDT_TRAIN_BRANCHES).values())).T

    dfs, output_br_names = process_directives(REGRESSION_BRANCHES, init_frame)
    regression_var = dfs[-1].AsNumpy(columns=['d0_et_diff'])

    if not args.load_bdt:
        print('Start training for a regression BDT...')
        time_start = time.perf_counter()
        rng = np.random.RandomState(1)
        bdt = AdaBoostRegressor(DecisionTreeRegressor(max_depth=args.max_depth),
                                n_estimators=300, random_state=rng)

        bdt_input = slice_bdt_input(input_vars)
        bdt.fit(bdt_input, regression_var['d0_et_diff'])
        time_stop = time.perf_counter()
        print('BDT fitted. It takes a total of {:,.2f} sec'.format(
            time_stop - time_start))

        if args.output != 'None':
            print('Export trained BDT to '+args.output)
            with open(args.output, 'wb') as f:
                pickle.dump(bdt, f)

    else:
        # Variables to add to new tree with BDT output
        idx_names = ['runNumber', 'eventNumber', 'd0_et_emu_no_bdt']
        idx_vars = np.array(
            list(init_frame.AsNumpy(columns=idx_names).values())).T
        output_tree = dict(zip(idx_names,idx_vars.T))

        # Loading BDT and calculating related variables
        print('Loading BDT from '+args.load_bdt)
        bdt = pickle.load(open(load_file(args.load_bdt), 'rb'))
        d0_et_corr = bdt.predict(input_vars)
        output_tree['d0_et_corr'] = d0_et_corr
        output_tree['d0_trg_et'] = dfs[-1].AsNumpy(columns=['d0_trg_et'])['d0_trg_et']
        bdt_rdf = ROOT.RDF.MakeNumpyDataFrame(output_tree)
        d0_et_emu_frame = bdt_rdf.Define('d0_et_emu', 'capHcalResp(d0_et_emu_no_bdt + d0_et_corr)')
        l0hadron_trg_frame = d0_et_emu_frame.Define('d0_l0_hadron_tos_emu',
        'l0HadronTriggerEmu(d0_et_emu, 2016)')

        # Saving tree
        branchKeep = ['runNumber', 'eventNumber','d0_l0_hadron_tos_emu', 'd0_et_corr', 'd0_et_emu','d0_trg_et']
        l0hadron_trg_frame.Snapshot(args.tree, args.output, branchKeep)
        print("Written "+args.output)
