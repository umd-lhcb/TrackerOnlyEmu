#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Tue Apr 20, 2021 at 03:15 AM +0200
# Stolen from: https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_L0GlobalTIS.py

from argparse import ArgumentParser

from ROOT import gInterpreter, RDataFrame

from TrackerOnlyEmu.loader import load_file, load_cpp
from TrackerOnlyEmu.executor import ExecDirective as EXEC
from TrackerOnlyEmu.executor import process_directives


#################################
# Command line arguments parser #
#################################

def parse_input():
    parser = ArgumentParser(
        description='Emulate L0Global TIS trigger.')

    parser.add_argument('input', help='''
specify input ntuple file.
''')

    parser.add_argument('output', help='''
specify output ntuple file.
''')

    parser.add_argument('-t', '--tree', default='TupleB0/DecayTree', help='''
specify tree name.
''')

    parser.add_argument('-y', '--year', default='2016', help='''
specify year.''')

    parser.add_argument('-B', '--Bmeson', default='b0', help='''
specify the name of the B meson in the tree.''')

    parser.add_argument('--debug', action='store_true', help='''
enable debug mode.
''')

    return parser.parse_args()


##########################################
# Load C++ definitions & ROOT histograms #
##########################################

load_cpp('<triggers/l0/run2-L0GlobalTIS.h>')

gInterpreter.Declare('auto histoResp = new TFile("{}");'.format(
    load_file('<triggers/l0/l0_tis_efficiency.root>')))

epilogue = '''
auto hResp = readL0GlobalTisResp(histoResp);
'''
gInterpreter.Declare(epilogue)


#################
# Apply trigger #
#################

if __name__ == '__main__':
    args = parse_input()

    directives = [
        EXEC('Define', '{}_true_pz'.format(args.Bmeson),
             '{}_TRUEP_Z'.format(args.Bmeson), True),
        EXEC('Define', '{}_true_pt'.format(args.Bmeson),
             '{}_TRUEPT'.format(args.Bmeson), True),
        EXEC('Define', '{}_l0_global_tis_emu'.format(args.Bmeson),
             'L0GlobalTisTriggerEmu({}, {}, {}, hResp)'.format(
                 '{}_true_pz'.format(args.Bmeson),
                 '{}_true_pt'.format(args.Bmeson),
                 args.year), True),
    ]

    directives_debug = [
        # Reference variables
        EXEC('Define', '{}_l0_global_tis'.format(args.Bmeson),
             '{}_L0Global_TIS', True),

        # Fit variables
        EXEC('Define', 'q2', 'FitVar_q2 / 1e6', True),
        EXEC('Define', 'mmiss2', 'FitVar_Mmiss2 / 1e6', True),
        EXEC('Define', 'el', 'FitVar_El / 1e3', True),
    ]

    if args.debug:
        directives += directives_debug

    init_frame = RDataFrame(args.tree, args.input)
    dfs, output_br_names = process_directives(directives, init_frame)

    # Always keep run and event numbers
    output_br_names.push_back('runNumber')
    output_br_names.push_back('eventNumber')

    # Output
    dfs[-1].Snapshot(args.tree, args.output, output_br_names)
