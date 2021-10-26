#!/usr/bin/env python3
#
# Author: Yipeng Sun
# License: BSD 2-clause
# Last Change: Tue Oct 26, 2021 at 02:51 PM +0200

from itertools import combinations

from TrackerOnlyEmu.loader import load_file, load_cpp
from TrackerOnlyEmu.executor import ExecDirective as EXEC

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

def func_call_gen(func, params, particle=None):
    if particle is not None:
        params = [particle+'_'+p for p in params]

    return '{}({})'.format(func, ', '.join(params))


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
