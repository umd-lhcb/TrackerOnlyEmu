#!/usr/bin/env python3
"""Produce the corrected-SPD<450 efficiency friend, independent of L0 triggers."""
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True
ROOT.PyConfig.DisableRootLogon = True

from argparse import ArgumentParser
from TrackerOnlyEmu.executor import process_directives
from TrackerOnlyEmu.emulation.run2_rdx import run2_rdx_nspdcor_lt450_directive_gen


def main():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument('input')
    parser.add_argument('output')
    parser.add_argument('-t', '--tree', default='TupleB0/DecayTree')
    parser.add_argument('-y', '--year', choices=('2016', '2017', '2018'), default='2016')
    parser.add_argument('--map', dest='map_file', default='<triggers/l0/nspdhits_corrected_lt450_Dstmunu_FS_eff_notrigger.root>')
    args = parser.parse_args()
    directives = run2_rdx_nspdcor_lt450_directive_gen(args.year, args.map_file)
    frame = ROOT.RDataFrame(args.tree, args.input)
    frames, branches = process_directives(directives, frame)
    branches.push_back('runNumber')
    branches.push_back('eventNumber')
    options = ROOT.RDF.RSnapshotOptions()
    options.fMode = 'UPDATE'
    frames[-1].Snapshot(args.tree, args.output, branches, options)


if __name__ == '__main__':
    main()
