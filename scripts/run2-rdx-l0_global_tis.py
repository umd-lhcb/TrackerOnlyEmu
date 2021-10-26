#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Tue Oct 26, 2021 at 04:02 PM +0200
# Stolen from: https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_L0GlobalTIS.py

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True  # Don't hijack argparse!
ROOT.PyConfig.DisableRootLogon = True  # Don't read .rootlogon.py

from argparse import ArgumentParser

from ROOT import RDataFrame

from TrackerOnlyEmu.executor import ExecDirective as EXEC
from TrackerOnlyEmu.executor import process_directives
from TrackerOnlyEmu.emulation.run2_rdx import \
    run2_rdx_l0_global_tis_directive_gen


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


#################
# Apply trigger #
#################

if __name__ == '__main__':
    args = parse_input()

    directives = run2_rdx_l0_global_tis_directive_gen(args.Bmeson, args.year)

    directives_debug = [
        # Reference variables
        EXEC('Define', '{}_l0_global_tis'.format(args.Bmeson),
             '{}_L0Global_TIS'.format(args.Bmeson), True),

        # Fit variables
        EXEC('Define', 'q2', 'FitVar_q2 / 1e6', True),
        EXEC('Define', 'mmiss2', 'FitVar_Mmiss2 / 1e6', True),
        EXEC('Define', 'el', 'FitVar_El / 1e3', True),

        # Log of B meson momenta
        EXEC('Define', 'log_{}_pz'.format(args.Bmeson),
             'TMath::Log({}_pz)'.format(args.Bmeson), True),
        EXEC('Define', 'log_{}_pt'.format(args.Bmeson),
             'TMath::Log({}_pt)'.format(args.Bmeson), True),
    ]

    if args.debug:
        directives += directives_debug
        # Apply the nSPDHits cut
        # directives.append(
        #     EXEC('Filter', instruct='NumSPDHits < 450'))
        directives.append(
            EXEC('Define', 'nspd_hits', 'NumSPDHits', True))

    init_frame = RDataFrame(args.tree, args.input)
    dfs, output_br_names = process_directives(directives, init_frame)

    # Always keep run and event numbers
    output_br_names.push_back('runNumber')
    output_br_names.push_back('eventNumber')

    # Output
    dfs[-1].Snapshot(args.tree, args.output, output_br_names)
