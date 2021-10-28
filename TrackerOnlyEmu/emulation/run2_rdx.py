#!/usr/bin/env python3
#
# Author: Yipeng Sun
# License: BSD 2-clause
# Last Change: Thu Oct 28, 2021 at 04:07 AM +0200

from itertools import combinations
from ROOT import gInterpreter

from TrackerOnlyEmu.loader import load_file, load_cpp
from TrackerOnlyEmu.executor import ExecDirective as EXEC
from TrackerOnlyEmu.utils import func_call_gen


######################
# L0 Hadron TOS: XGB #
######################
# Configurables ################################################################

XGB_TRAIN_BRANCHES = [
    'nTracks',  ## To model the NumSPDHits < 450 cut
    'd0_P',
    'd0_PT',
    'k_P',
    'k_PT',
    'k_L0Calo_HCAL_realET',
    'k_L0Calo_HCAL_xProjection',
    'k_L0Calo_HCAL_yProjection',
    'k_L0Calo_HCAL_region',
    'pi_P',
    'pi_PT',
    'pi_L0Calo_HCAL_realET',
    'pi_L0Calo_HCAL_xProjection',
    'pi_L0Calo_HCAL_yProjection',
    'pi_L0Calo_HCAL_region',
    # 'spi_P',
    # 'spi_PT',
    # 'spi_L0Calo_HCAL_realET',
    # 'spi_L0Calo_HCAL_xProjection',
    # 'spi_L0Calo_HCAL_yProjection',
    # 'spi_L0Calo_HCAL_region',
]


##############################
# L0 Hadron TOS: RD+, no BDT #
##############################
# Configurables ################################################################

BDT_TRAIN_BRANCHES = [
    'd0_PT',
    'd0_P',
    'k_L0Calo_HCAL_realET',  # NOTE: realET is not Trigger ET!
    'pi_L0Calo_HCAL_realET',
    'rdiff_k_pi'  # NOTE: This has to be computed by us!
]


# Main #########################################################################

def run2_rdx_l0_hadron_tos_no_bdt_directive_gen(year):
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

    return [
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
             'static_cast<Double_t>(l0HadronTriggerEmu(d0_et_emu_no_bdt, {}))'.format(year),
             True),
        EXEC('Define', 'd0_trg_et',
             'capHcalResp(k_L0Calo_HCAL_TriggerET, pi_L0Calo_HCAL_TriggerET)',
             True),
        EXEC('Define', 'd0_et_diff', 'd0_trg_et - d0_et_emu_no_bdt',
             True),
    ]


#################
# L0 Global TIS #
#################
# Main #########################################################################

def run2_rdx_l0_global_tis_directive_gen(Bmeson, year):
    load_cpp('<triggers/l0/run2-L0GlobalTIS.h>')

    gInterpreter.Declare('auto histoResp = new TFile("{}");'.format(
        load_file('<triggers/l0/l0_tis_efficiency.root>')))

    epilogue = '''
    auto hResp = readL0GlobalTisResp(histoResp);
    '''
    gInterpreter.Declare(epilogue)

    return [
        EXEC('Define', '{}_pz'.format(Bmeson),
             '{}_PZ'.format(Bmeson), True),
        EXEC('Define', '{}_pt'.format(Bmeson),
             '{}_PT'.format(Bmeson), True),
        EXEC('Define', '{}_l0_global_tis_emu'.format(Bmeson),
             'l0GlobalTisTriggerEmu({}, {}, {}, hResp)'.format(
                 '{}_pz'.format(Bmeson),
                 '{}_pt'.format(Bmeson),
                 year), True),
    ]


#########
# HLT 1 #
#########
# Configurables ################################################################

GEC_SEL_BRANCHES = [
    'NumVeloClusters',
    'NumITClusters',
    'NumOTClusters',
]

GLOBAL_CORR_BRANCHES = {
    'particle': [
        'TRACK_nTTHits'
    ],
    'global': [
        'NumVeloClusters',
        'NumITClusters',
        'NumOTClusters',
    ]
}

TRACK_SEL_BRANCHES = [
    'PT',
    'P',
    'TRACK_CHI2NDOF',
    'IPCHI2_OWNPV',
    'TRACK_GhostProb'
]

TWO_TRACK_SPEC_BRANCHES = {
    'PT':  'PT',
    'P': 'P',
    'TRCHI2DOF': 'TRACK_CHI2NDOF',
    'BPVIPCHI2': 'IPCHI2_OWNPV',
    'TRGHOSTPROB': 'TRACK_GhostProb',
    'PX': 'PX',
    'PY': 'PY',
}

TWO_TRACK_COMB_SPEC_BRANCHES = {
    'VDCHI2': 'VDCHI2_OWNPV_COMB',
    'SUMPT': 'SUMPT_COMB',
    'DOCA': 'DOCA_COMB',
    'VCHI2': 'VERTEX_CHI2_COMB',
    'BPVETA': 'ETA_COMB',
    'BPVCORRM': 'MCORR_OWNPV_COMB',
    'BPVDIRA': 'DIRA_OWNPV_COMB',
    'MVA': 'Matrixnet_Hlt1TwoTrackMVAEmulations',
}


# Helpers ######################################################################

def global_corr_gen(particle):
    params = [particle+'_'+b for b in GLOBAL_CORR_BRANCHES['particle']]
    params += GLOBAL_CORR_BRANCHES['global']
    return 'hlt1GlobalPass({})'.format(', '.join(params))


def track_spec_gen(particles, branches):
    specs = []

    for p in particles:
        spec = []
        for name, val in branches.items():
            spec.append('{' + '"{}"'.format(name) + ', ' + p + '_' + val  + '}')

        specs.append('{' + ', '.join(spec) + '}')

    return 'vector<map<string, double> >{' + ', '.join(specs) + '}'


def comb_spec_gen(particle, branches, suffixs):
    specs = []

    for suf1, suf2 in combinations(suffixs, 2):
        spec = []
        for name, val in branches.items():
            spec.append(
                '{' + '"{}"'.format(name) + ', ' + particle + '_' + val +
                '_' + '{}_{}'.format(suf1, suf2) + '}')

        specs.append('{' + ', '.join(spec) + '}')

    return 'vector<map<string, double> >{' + ', '.join(specs) + '}'


# Main #########################################################################

def run2_rdx_hlt1_directive_gen(Bmeson, year):
    load_cpp('<triggers/hlt1/run2-Hlt1GEC.h>')
    load_cpp('<triggers/hlt1/run2-Hlt1TrackMVA.h>')
    load_cpp('<triggers/hlt1/run2-Hlt1TwoTrackMVA.h>')
    load_cpp('<triggers/kinematics.h>')

    return [
        # Various corrections
        EXEC('Define', 'pass_gec',
             func_call_gen('hlt1GEC', GEC_SEL_BRANCHES), True),
        EXEC('Define', 'k_pass_hlt1_corr', global_corr_gen('k'), True),
        EXEC('Define', 'pi_pass_hlt1_corr', global_corr_gen('pi'), True),

        # Hlt1TrackMVA emulation
        EXEC('Define', 'k_hlt1_trackmva_tos_emu',
             func_call_gen(
                 'hlt1TrackMVATriggerEmu',
                 ['k_'+n for n in TRACK_SEL_BRANCHES] +
                 ['k_pass_hlt1_corr', year]), True),
        EXEC('Define', 'pi_hlt1_trackmva_tos_emu',
             func_call_gen(
                 'hlt1TrackMVATriggerEmu',
                 ['pi_'+n for n in TRACK_SEL_BRANCHES] +
                 ['pi_pass_hlt1_corr', year]), True),
        EXEC('Define', 'd0_hlt1_trackmva_tos_emu',
             'k_hlt1_trackmva_tos_emu || pi_hlt1_trackmva_tos_emu', True),

        # Hlt1TwoTrackMVA emulation
        EXEC('Define', 'vec_pass_hlt1_corr',
             'vector<bool>{ k_pass_hlt1_corr, pi_pass_hlt1_corr }'),
        EXEC('Define', 'track_spec',
             track_spec_gen(['k', 'pi'], TWO_TRACK_SPEC_BRANCHES)),
        EXEC('Define', 'comb_spec',
             comb_spec_gen(Bmeson,
                           TWO_TRACK_COMB_SPEC_BRANCHES, range(1, 4))),
        EXEC('Define', 'd0_hlt1_twotrackmva_tos_emu',
             'hlt1TwoTrackMVATriggerEmu(track_spec, comb_spec, vec_pass_hlt1_corr, {})'.format(year),
             True),
    ]
