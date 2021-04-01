#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Fri Apr 02, 2021 at 01:03 AM +0200

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

GLOBAL_CORR_BRANCHES = [
    'NumTTHits',  # FIXME Variable name!
    'NumVeloClusters',
    'NumITClusters',
    'NumOTClusters',
]

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

    parser.add_argument('-t', '--tree', default='TupleB0/DecayTree',
                        help='''
specify tree name.
''')

    parser.add_argument('-y', '--year', default='2016',
                        help='''
specify year.''')

    return parser.parse_args()


########################
# Load C++ definitions #
########################

load_cpp('<triggers/hlt1/run2-Hlt1GEC.h>')
load_cpp('<triggers/hlt1/run2-Hlt1TrackMVA.h>')
load_cpp('<triggers/hlt1/run2-Hlt1TwoTrackMVA.h>')


##################
# Apply triggers #
##################

def func_call_gen(func, params, particle=None):
    if particle is not None:
        params = [particle+'_'+p for p in params]

    return '{}({})'.format(func, ', '.join(params))


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
        EXEC('Define', 'pass_gec',
             func_call_gen('hlt1GEC', GEC_SEL_BRANCHES), True),

        # Hlt1TrackMVA emulation
        EXEC('Define', 'k_hlt1_trackmva_tos',
             func_call_gen(
                 'hlt1TrackMVATriggerEmu',
                 ['k_'+n for n in TRACK_SEL_BRANCHES] + [args.year]), True),
        EXEC('Define', 'pi_hlt1_trackmva_tos',
             func_call_gen(
                 'hlt1TrackMVATriggerEmu',
                 ['pi_'+n for n in TRACK_SEL_BRANCHES] + [args.year]), True),

        # Hlt1TwoTrackMVA emulation
        EXEC('Define', 'vec_pass_gec',
             'vector<bool>{ pass_gec, pass_gec }'),
        EXEC('Define', 'track_spec',
             track_spec_gen(['k', 'pi'], TWO_TRACK_SPEC_BRANCHES)),
        EXEC('Define', 'comb_spec',
             comb_spec_gen('b0', TWO_TRACK_COMB_SPEC_BRANCHES, range(1, 4))),
        EXEC('Define', 'd0_hlt1_twotrackmva_tos_gec',
             'hlt1TwoTrackMVATriggerEmu(track_spec, comb_spec, vec_pass_gec, {})'.format(args.year),
             True),

        # Debug: TwoTrackMVA input track cuts
        # EXEC('Define', 'k_diff_chi2ndof',
        #      'b0_TRACK_CHI2_DAU_1/b0_TRACK_NDOF_DAU_1 - k_TRACK_CHI2NDOF',
        #       True),
        # EXEC('Define', 'pi_diff_chi2ndof',
        #      'b0_TRACK_CHI2_DAU_2/b0_TRACK_NDOF_DAU_2 - pi_TRACK_CHI2NDOF',
        #      True),

        # EXEC('Define', 'k_diff_p', 'b0_P_DAU_1 - k_P', True),
        # EXEC('Define', 'pi_diff_p', 'b0_P_DAU_2 - pi_P', True),

        # EXEC('Define', 'k_diff_pt', 'b0_PT_DAU_1 - k_PT', True),
        # EXEC('Define', 'pi_diff_pt', 'b0_PT_DAU_2 - pi_PT', True),

        # EXEC('Define', 'k_diff_ipchi2',
        #      'b0_IPCHI2_OWNPV_DAU_1 - k_IPCHI2_OWNPV', True),
        # EXEC('Define', 'pi_diff_ipchi2',
        #      'b0_IPCHI2_OWNPV_DAU_2 - pi_IPCHI2_OWNPV', True),
    ]

    init_frame = RDataFrame(args.tree, args.input)
    dfs, output_br_names = process_directives(directives, init_frame)

    # Always keep run and event numbers
    output_br_names.push_back('runNumber')
    output_br_names.push_back('eventNumber')

    # Output
    dfs[-1].Snapshot(args.tree, args.output, output_br_names)
