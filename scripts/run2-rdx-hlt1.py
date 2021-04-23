#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Fri Apr 23, 2021 at 03:21 AM +0200

from argparse import ArgumentParser
from itertools import combinations

from ROOT import RDataFrame

from TrackerOnlyEmu.loader import load_cpp
from TrackerOnlyEmu.executor import ExecDirective as EXEC
from TrackerOnlyEmu.executor import process_directives


#################
# Configurables #
#################

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
    'TRCHOSTPROB': 'TRACK_GhostProb',
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


#################################
# Command line arguments parser #
#################################

def parse_input():
    parser = ArgumentParser(
        description='Emulate Hlt1{Track,TwoTrack}MVA triggers.')

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


########################
# Load C++ definitions #
########################

load_cpp('<triggers/hlt1/run2-Hlt1GEC.h>')
load_cpp('<triggers/hlt1/run2-Hlt1TrackMVA.h>')
load_cpp('<triggers/hlt1/run2-Hlt1TwoTrackMVA.h>')
load_cpp('<triggers/kinematics.h>')


##################
# Apply triggers #
##################

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


if __name__ == '__main__':
    args = parse_input()

    directives = [
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
                 ['k_pass_hlt1_corr', args.year]), True),
        EXEC('Define', 'pi_hlt1_trackmva_tos_emu',
             func_call_gen(
                 'hlt1TrackMVATriggerEmu',
                 ['pi_'+n for n in TRACK_SEL_BRANCHES] +
                 ['pi_pass_hlt1_corr', args.year]), True),
        EXEC('Define', 'd0_hlt1_trackmva_tos_emu',
             'k_hlt1_trackmva_tos_emu || pi_hlt1_trackmva_tos_emu', True),

        # Hlt1TwoTrackMVA emulation
        EXEC('Define', 'vec_pass_hlt1_corr',
             'vector<bool>{ k_pass_hlt1_corr, pi_pass_hlt1_corr }'),
        EXEC('Define', 'track_spec',
             track_spec_gen(['k', 'pi'], TWO_TRACK_SPEC_BRANCHES)),
        EXEC('Define', 'comb_spec',
             comb_spec_gen(args.Bmeson,
                           TWO_TRACK_COMB_SPEC_BRANCHES, range(1, 4))),
        EXEC('Define', 'd0_hlt1_twotrackmva_tos_emu',
             'hlt1TwoTrackMVATriggerEmu(track_spec, comb_spec, vec_pass_hlt1_corr, {})'.format(args.year),
             True),
    ]

    directives_debug = [
        # Reference variables
        EXEC('Define', 'k_l0_global_dec', 'k_L0Global_Dec', True),
        EXEC('Define', 'k_hlt1_trackmva_tos', 'k_Hlt1TrackMVADecision_TOS',
             True),
        EXEC('Define', 'pi_l0_global_dec', 'pi_L0Global_Dec', True),
        EXEC('Define', 'pi_hlt1_trackmva_tos', 'pi_Hlt1TrackMVADecision_TOS',
             True),
        EXEC('Define', 'd0_l0_global_dec', 'd0_L0Global_Dec', True),
        EXEC('Define', 'd0_hlt1_trackmva_tos', 'd0_Hlt1TrackMVADecision_TOS',
             True),
        EXEC('Define', 'd0_hlt1_twotrackmva_tos',
             'd0_Hlt1TwoTrackMVADecision_TOS', True),

        # Fit variables
        EXEC('Define', 'q2', 'FitVar_q2 / 1e6', True),
        EXEC('Define', 'mmiss2', 'FitVar_Mmiss2 / 1e6', True),
        EXEC('Define', 'el', 'FitVar_El / 1e3', True),

        # Kinematic variables
        EXEC('Define', 'k_pt', 'k_PT / 1e3', True),
        EXEC('Define', 'k_p', 'k_P / 1e3', True),
        EXEC('Define', 'pi_pt', 'pi_PT / 1e3', True),
        EXEC('Define', 'pi_p', 'pi_P / 1e3', True),
        EXEC('Define', 'k_pi_apt', 'computePt(k_PX+pi_PX, k_PY+pi_PY) / 1e3',
             True),

        # Track quality variables
        EXEC('Define', 'k_chi2ndof', 'k_TRACK_CHI2NDOF', True),
        EXEC('Define', 'k_ipchi2', 'k_IPCHI2_OWNPV', True),
        EXEC('Define', 'k_ghost', 'k_TRACK_GhostProb', True),
        EXEC('Define', 'pi_chi2ndof', 'pi_TRACK_CHI2NDOF', True),
        EXEC('Define', 'pi_ipchi2', 'pi_IPCHI2_OWNPV', True),
        EXEC('Define', 'pi_ghost', 'pi_TRACK_GhostProb', True),

        # Angular variables
        EXEC('Define', 'mu_theta', 'theta(mu_PZ, mu_P)', True),
        EXEC('Define', 'k_theta', 'theta(k_PZ, k_P)', True),
        EXEC('Define', 'pi_theta', 'theta(pi_PZ, pi_P)', True),
        EXEC('Define', 'mu_phi', 'phi(mu_PX, mu_PY)', True),
        EXEC('Define', 'k_phi', 'phi(k_PX, k_PY)', True),
        EXEC('Define', 'pi_phi', 'phi(pi_PX, pi_PY)', True),
    ]

    directives_mva = []
    for i, j in combinations(range(1, 5), 2):
        directives_mva.append(
            EXEC('Define', 'mva_score_{}_{}'.format(i, j),
            '{}_Matrixnet_Hlt1TwoTrackMVAEmulations_{}_{}'.format(
                args.Bmeson, i, j), True))
        directives_mva.append(
            EXEC('Define', 'mva_dira_{}_{}'.format(i, j),
            '{}_DIRA_OWNPV_COMB_{}_{}'.format(args.Bmeson, i, j), True))
        directives_mva.append(
            EXEC('Define', 'mva_doca_{}_{}'.format(i, j),
            '{}_DOCA_COMB_{}_{}'.format(args.Bmeson, i, j), True))
        directives_mva.append(
            EXEC('Define', 'mva_eta_{}_{}'.format(i, j),
            '{}_ETA_COMB_{}_{}'.format(args.Bmeson, i, j), True))
        directives_mva.append(
            EXEC('Define', 'mva_ip_chi2_{}_{}'.format(i, j),
            '{}_IPCHI2_OWNPV_COMB_{}_{}'.format(args.Bmeson, i, j), True))
        directives_mva.append(
            EXEC('Define', 'mva_pt_{}_{}'.format(i, j),
            '{}_PT_COMB_{}_{}'.format(args.Bmeson, i, j), True))
        directives_mva.append(
            EXEC('Define', 'mva_p_{}_{}'.format(i, j),
            '{}_P_COMB_{}_{}'.format(args.Bmeson, i, j), True))
        directives_mva.append(
            EXEC('Define', 'mva_vd_chi2_{}_{}'.format(i, j),
            '{}_VDCHI2_OWNPV_COMB_{}_{}'.format(args.Bmeson, i, j), True))
        directives_mva.append(
            EXEC('Define', 'mva_vertex_chi2_{}_{}'.format(i, j),
            '{}_VERTEX_CHI2_COMB_{}_{}'.format(args.Bmeson, i, j), True))
        directives_mva.append(
            EXEC('Define', 'mva_vertex_ndof_{}_{}'.format(i, j),
            '{}_VERTEX_NDOF_COMB_{}_{}'.format(args.Bmeson, i, j), True))

    if args.debug:
        directives += directives_debug
        directives += directives_mva

    init_frame = RDataFrame(args.tree, args.input)
    dfs, output_br_names = process_directives(directives, init_frame)

    # Always keep run and event numbers
    output_br_names.push_back('runNumber')
    output_br_names.push_back('eventNumber')

    # Output
    dfs[-1].Snapshot(args.tree, args.output, output_br_names)
