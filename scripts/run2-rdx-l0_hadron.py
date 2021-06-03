#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Thu Jun 03, 2021 at 08:58 PM +0200
# Stolen from: https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_L0Hadron_TOS_RLc.py

import pickle
import numpy as np

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True  # Don't hijack argparse!
ROOT.PyConfig.DisableRootLogon = True  # Don't read .rootlogon.py

from argparse import ArgumentParser

from ROOT import gInterpreter, RDataFrame

from TrackerOnlyEmu.loader import load_file, load_cpp
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

bdt = pickle.load(open(load_file('<triggers/l0/bdt.pickle>'), 'rb'))

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
        EXEC('Define', 'rdiff_k_pi', 'rDiff({})'.format(
            ', '.join([p+'_L0Calo_HCAL_'+d+'Projection'
                       for p in ['k', 'pi'] for d in ['x', 'y']])),
             True),
        EXEC('Define', 'shared_k_pi',
             'isShared(rdiff_k_pi, k_L0Calo_HCAL_region, pi_L0Calo_HCAL_region, hSharedIn, hSharedOut)',
             True),
        EXEC('Define', 'miss_k_pi',
             'missingFraction(rdiff_k_pi, k_L0Calo_HCAL_region, pi_L0Calo_HCAL_region, hMissIn, hMissOut)',
             True),
        EXEC('Define', 'k_et_emu_no_bdt',
             'twoPartEt(k_et_smeared, pi_et_smeared, shared_k_pi, miss_k_pi)',
             True),
        EXEC('Define', 'pi_et_emu_no_bdt',
             'twoPartEt(pi_et_smeared, k_et_smeared, shared_k_pi, miss_k_pi)',
             True),
        EXEC('Define', 'd0_et_emu_no_bdt',
             'TMath::Max(k_et_emu_no_bdt, pi_et_emu_no_bdt)', True),
        EXEC('Define', 'd0_l0_hadron_tos_emu_no_bdt',
             'l0HadronTriggerEmu(d0_et_emu_no_bdt, {})'.format(args.year),
             True),
    ]

    directives_debug = [
        # Reference variables
        EXEC('Define', 'd0_l0_hadron_tos', 'd0_L0HadronDecision_TOS', True),

        # Fit variables
        EXEC('Define', 'q2', 'FitVar_q2 / 1e6', True),
        EXEC('Define', 'mmiss2', 'FitVar_Mmiss2 / 1e6', True),
        EXEC('Define', 'el', 'FitVar_El / 1e3', True),

        # Kinematic variables
        EXEC('Define', 'd0_pt', 'd0_PT / 1e3', True),
        EXEC('Define', 'k_pt', 'k_PT / 1e3', True),
        EXEC('Define', 'pi_pt', 'pi_PT / 1e3', True),
        EXEC('Define', 'd0_pt_raw', 'd0_PT', True),
        EXEC('Define', 'k_pt_raw', 'k_PT', True),
        EXEC('Define', 'pi_pt_raw', 'pi_PT', True),
        EXEC('Define', 'rdiff_k_pi_wrong', 'rDiff(k_X, k_Y, pi_X, pi_Y)', True),

        # Cut variable candidates
        EXEC('Define', 'k_trg_et', 'k_L0Calo_HCAL_TriggerET', True),
        EXEC('Define', 'k_trg_hcal_et', 'k_L0Calo_HCAL_TriggerHCALET', True),
        EXEC('Define', 'pi_trg_et', 'pi_L0Calo_HCAL_TriggerET', True),
        EXEC('Define', 'pi_trg_hcal_et', 'pi_L0Calo_HCAL_TriggerHCALET', True),
        EXEC('Define', 'k_pi_trg_et_sum',
             'capHcalResp(k_L0Calo_HCAL_TriggerET+ pi_L0Calo_HCAL_TriggerET)',
             True),
        EXEC('Define', 'k_pi_trg_hcal_et_sum',
             'capHcalResp(k_L0Calo_HCAL_TriggerHCALET+pi_L0Calo_HCAL_TriggerHCALET)',
             True),
        EXEC('Define', 'k_pi_trg_et_cap',
             'capHcalResp(k_L0Calo_HCAL_TriggerET, pi_L0Calo_HCAL_TriggerET)',
             True),
    ]

    if args.debug:
        directives += directives_debug
        # Apply the nSPDHits cut
        directives.append(
            EXEC('Filter', instruct='NumSPDHits < 450'))
        directives.append(
            EXEC('Define', 'nspd_hits', 'NumSPDHits', True))

    init_frame = RDataFrame(args.tree, args.input)
    dfs, output_br_names = process_directives(directives, init_frame)

    # Apply regression BDT for final HCAL ET correction
    bdt_input_vars = np.array(
        list(dfs[-1].AsNumpy(columns=BDT_TRAIN_BRANCHES).values())).T
    d0_et_corr = bdt.predict(bdt_input_vars)

    # Always keep run and event numbers
    output_br_names.push_back('runNumber')
    output_br_names.push_back('eventNumber')

    # Output
    output_vars = dfs[-1].AsNumpy(columns=[str(s) for s in output_br_names])
    output_vars['d0_et_corr'] = d0_et_corr

    # Need to enforce 'bool' type for branches like 'shared_k_pi
    for br, arr in output_vars.items():
        if arr.dtype == np.dtype('O'):
            arr = arr.astype(int)
            output_vars[br] = arr

    bdt_frame = ROOT.RDF.MakeNumpyDataFrame(output_vars)

    # HCAL ET and L0Hadron trigger w/ all corrections
    d0_et_emu_frame = bdt_frame.Define(
        'd0_et_emu', 'capHcalResp(d0_et_emu_no_bdt + d0_et_corr)')
    l0hadron_trg_frame = d0_et_emu_frame.Define(
        'd0_l0_hadron_tos_emu',
        'l0HadronTriggerEmu(d0_et_emu, {})'.format(args.year))

    # Keep the HCAL ET w/ BDT corrections
    output_br_names.push_back('d0_et_corr')
    output_br_names.push_back('d0_et_emu')
    output_br_names.push_back('d0_l0_hadron_tos_emu')

    l0hadron_trg_frame.Snapshot(args.tree, args.output, output_br_names)
