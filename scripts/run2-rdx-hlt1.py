#!/usr/bin/env python
#
# Author: Yipeng Sun
# Last Change: Mon Mar 29, 2021 at 04:13 AM +0200

from ROOT import TFile, TTree, gInterpreter, RDataFrame
from ROOT.std import vector

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

    # Debug
    df3 = df2.Define('hlt1_two_trk_mva_tos',
                     'hlt1TwoTrackMVATriggerEmu(track_spec, comb_spec, 2016)')

    df4 = df3.Define('comb_trigger_1_2',
                     'hlt1TwoTrackMVADec(b0_VDCHI2_OWNPV_COMB_1_2, b0_SUMPT_COMB_1_2, b0_VERTEX_CHI2_COMB_1_2, b0_ETA_COMB_1_2, b0_MCORR_OWNPV_COMB_1_2, b0_DIRA_OWNPV_COMB_1_2, b0_Matrixnet_Hlt1TwoTrackMVAEmulations_1_2, 2016)')

    df5 = df4.Define('track_trigger_1',
                     'hlt1TwoTrackInputDec(k_PT, k_P, k_TRACK_CHI2NDOF, k_IPCHI2_OWNPV, k_TRACK_GhostProb, 2016)')
    df6 = df5.Define('track_trigger_2',
                     'hlt1TwoTrackInputDec(pi_PT, pi_P, pi_TRACK_CHI2NDOF, pi_IPCHI2_OWNPV, pi_TRACK_GhostProb, 2016)')

    df7 = df6.Define('track_trigger_3',
                     'hlt1TwoTrackInputDec(spi_PT, spi_P, spi_TRACK_CHI2NDOF, spi_IPCHI2_OWNPV, spi_TRACK_GhostProb, 2016)')

    output_branch_names = vector('string')()
    output_branch_names.push_back('hlt1_two_trk_mva_tos')
    output_branch_names.push_back('comb_trigger_1_2')
    output_branch_names.push_back('track_trigger_1')
    output_branch_names.push_back('track_trigger_2')
    output_branch_names.push_back('track_trigger_3')

    # Output
    df7.Snapshot('tree', args.output, output_branch_names)
