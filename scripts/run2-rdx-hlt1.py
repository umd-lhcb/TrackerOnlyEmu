#!/usr/bin/env python
#
# Author: Yipeng Sun
# Last Change: Mon Mar 29, 2021 at 03:45 AM +0200

from ROOT import TFile, TTree, gInterpreter, RDataFrame

from argparse import ArgumentParser
from itertools import combinations


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

with open('../triggers/hlt1/run2-Hlt1TwoTrackMVA.h', 'r') as f:
    hlt1TwoTrackMVAHeader = f.read()

gInterpreter.Declare(hlt1TwoTrackMVAHeader)


##################
# Apply triggers #
##################

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

    if args.output is not None:
        ntp = TFile.Open(args.ntp, 'read')
    else:
        ntp = TFile.Open(args.ntp, 'update')

    print(args.tree)
    tree = ntp.Get(args.tree)

    df0 = RDataFrame(tree)
    df1 = df0.Define(
        'track_spec', track_spec_gen(
            ['k', 'pi', 'spi'],
            {'PT':  'PT',
             'P': 'P',
             'TRCHI2DOF': 'TRACK_CHI2NDOF',
             'BPVIPCHI2': 'IPCHI2_OWNPV',
             'TRCHOSTPROB': 'TRACK_GhostProb'}
        ))
    df2 = df1.Define(
        'comb_spec', comb_spec_gen(
            'b0',
            {'VDCHI2': 'VDCHI2_OWNPV_COMB',
             'SUMPT': 'SUMPT_COMB',
             'VCHI2': 'VERTEX_CHI2_COMB',
             'BPVETA': 'ETA_COMB',
             'BPVCORRM': 'MCORR_OWNPV_COMB',
             'DIRA': 'DIRA_OWNPV_COMB',
             'MVA': 'Matrixnet_Hlt1TwoTrackMVAEmulations'},
            range(1, 4)
        )
    )
