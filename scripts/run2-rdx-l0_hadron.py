#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Mon Apr 19, 2021 at 03:41 AM +0200
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

epilogue = '''
auto hResp = readSinglePartResp(histoResp);

auto hSharedIn  = static_cast<TH1D*>(histoCluster->Get("shared_with_radial_inner"));
auto hSharedOut = static_cast<TH1D*>(histoCluster->Get("shared_with_radial_outer"));

auto hMissIn  = static_cast<TH1D*>(histoCluster->Get("missing_with_radial_inner"));
auto hMissOut = static_cast<TH1D*>(histoCluster->Get("missing_with_radial_outer"));
'''
gInterpreter.Declare(epilogue)


#################
# Apply trigger #
#################

if __name__ == '__main__':
    args = parse_input()

    directives = [
        EXEC('Define', 'k_et_smeared',
             'singlePartEt(k_P, k_PT, k_L0Calo_HCAL_realET, hResp)', True),
        EXEC('Define', 'pi_et_smeared',
             'singlePartEt(pi_P, pi_PT, pi_L0Calo_HCAL_realET, hResp)', True),
        EXEC('Define', 'rdiff_k_pi', 'rDiff(k_X, k_Y, pi_X, pi_Y)', True),
        EXEC('Define', 'shared_k_pi',
             'isShared(rdiff_k_pi, k_L0Calo_HCAL_region, pi_L0Calo_HCAL_region, hSharedIn, hSharedOut)',
             True),
        EXEC('Define', 'miss_k_pi',
             'missingFraction(rdiff_k_pi, k_L0Calo_HCAL_region, pi_L0Calo_HCAL_region, hMissIn, hMissOut)',
             True),
        EXEC('Define', 'k_et_emu',
             'twoPartEt(k_et_smeared, pi_et_smeared, shared_k_pi, miss_k_pi)',
             True),
        EXEC('Define', 'pi_et_emu',
             'twoPartEt(pi_et_smeared, k_et_smeared, shared_k_pi, miss_k_pi)',
             True),
        EXEC('Define', 'd0_et_emu', 'TMath::Max(k_et_emu, pi_et_emu)', True),
        EXEC('Define', 'd0_l0_hadron_tos_emu',
             'L0Emu(d0_et_emu, {})'.format(args.year), True),
    ]

    directives_debug = [
        # Reference variables
        EXEC('Define', 'd0_l0_hadron_tos', 'd0_L0HadronDecision_TOS', True)
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
