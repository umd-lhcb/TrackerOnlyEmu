#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Mon Apr 19, 2021 at 03:20 AM +0200
# Stolen from: https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_L0Hadron_TOS_RLc.py

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
        description='Emulate L0Hadron trigger.')

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

    parser.add_argument('--debug', action='store_true', help='''
enable debug mode.
''')

    return parser.parse_args()


##########################################
# Load C++ definitions & ROOT histograms #
##########################################

load_cpp('<triggers/l0/run2-L0Hadron.h>')

gInterpreter.Declare('auto histoResp = new TFile("{}");'.format(
    load_file('<triggers/l0/hcal_et_response.root>')))
gInterpreter.Declare('auto histoCluster = new TFile("{}");'.format(
    load_file('<triggers/l0/hcal_two_part_clusters.root>')))

two_part_histos = '''
auto hMissIn  = static_cast<TH1D*>(histoCluster->Get("missing_with_radial_inner"));
auto hMissOut = static_cast<TH1D*>(histoCluster->Get("missing_with_radial_outer"));

auto hSharedIn  = static_cast<TH1D*>(histoCluster->Get("shared_with_radial_inner"));
auto hSharedOut = static_cast<TH1D*>(histoCluster->Get("shared_with_radial_outer"));
'''
gInterpreter.Declar(two_part_histos)
