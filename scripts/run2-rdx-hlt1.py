#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Thu Apr 01, 2021 at 12:18 AM +0200

from argparse import ArgumentParser
from itertools import combinations

from ROOT import TFile, TTree, RDataFrame

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

TWO_TRACK_SPEC_BRANCHES = {
    'PT':  'PT',
    'P': 'P',
    'TRCHI2DOF': 'TRACK_CHI2NDOF',
    'BPVIPCHI2': 'IPCHI2_OWNPV',
    'TRCHOSTPROB': 'TRACK_GhostProb',
}

TWO_TRACK_COMB_SPEC_BRANCHES = {
    'VDCHI2': 'VDCHI2_OWNPV_COMB',
    'SUMPT': 'SUMPT_COMB',
    'VCHI2': 'VERTEX_CHI2_COMB',
    'BPVETA': 'ETA_COMB',
    'BPVCORRM': 'MCORR_OWNPV_COMB',
    'BPVDIRA': 'DIRA_OWNPV_COMB',
    'MVA': 'Matrixnet_Hlt1TwoTrackMVAEmulations',
    'DOCA': 'DOCA_COMB',
}


#################################
# Command line arguments parser #
#################################

def parse_input():
    parser = ArgumentParser(
        description='Apply Hlt1{Track,TwoTrack}MVA triggers.')

    parser.add_argument('ntp', help='''
specify input ntuple file.
''')

    parser.add_argument('-t', '--tree', default='TupleB0/DecayTree',
                        help='''
specify tree name.
''')

    parser.add_argument('-o', '--output', default=None,
                        help='''
specify output ntuple file (optional).
''')

    return parser.parse_args()


########################
# Load C++ definitions #
########################

load_cpp('<triggers/hlt1/run2-Hlt1GEC.h>')
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


directives = [
    EXEC('Define', 'track_spec',
         track_spec_gen(['k', 'pi'], TWO_TRACK_SPEC_BRANCHES)),
    EXEC('Define', 'comb_spec',
         comb_spec_gen('b0', TWO_TRACK_COMB_SPEC_BRANCHES, range(1, 4))),
    EXEC('Define', 'pass_gec',
         func_call_gen('hlt1GEC', GEC_SEL_BRANCHES), True),
    EXEC('Define', 'vec_pass_gec',
         'vector<bool>{pass_gec, pass_gec}'),
    EXEC('Define', 'd0_Hlt1TwoTrackMVA_TOS',
         'hlt1TwoTrackMVATriggerEmu(track_spec, comb_spec, vec_pass_gec, 2016)',
         True)
]


if __name__ == '__main__':
    args = parse_input()

    if args.output is not None:
        ntp = TFile.Open(args.ntp, 'read')
    else:
        ntp = TFile.Open(args.ntp, 'update')

    tree = ntp.Get(args.tree)
    dfs, output_br_names = process_directives(directives, RDataFrame(tree))

    # Output
    dfs[-1].Snapshot('tree', args.output, output_br_names)
