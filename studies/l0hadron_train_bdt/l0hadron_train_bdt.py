#!/usr/bin/env python
#
# Script to train BDTs and produce training, validation and
# non-truth-matched (ntm) ntuples with the BDT output

import os
import pathlib

def run_cmd(cmd):
    print('\n \033[92m'+cmd+'\033[0m')
    # os.system(cmd)

def copyFile(srcFile, outFile):
    if not pathlib.Path(outFile).is_file():
        run_cmd('cp '+srcFile+' '+outFile)

## Copying source ntuples from lhcb-ntuples-gen if they don't exist
srcFolder = '../../../lhcb-ntuples-gen/studies/ntuple-RDX_l0_hadron_tos_training_sample/'
ntpFolder = '../../samples/'
ntpTrain = 'l0hadron_emu_tm_train.root'
ntpValid = 'l0hadron_emu_tm_valid.root'
ntpNtm = 'l0hadron_emu_ntm.root'
copyFile(srcFolder+ntpTrain, ntpFolder+ntpTrain)
copyFile(srcFolder+ntpValid, ntpFolder+ntpValid)
copyFile(srcFolder+ntpNtm, ntpFolder+ntpNtm)

## Training BDTs
bdtEx = '../../scripts/lohadron_trainload_bdt.py '
run_cmd(bdtEx+ntpFolder+ntpTrain+' -o bdt4.pickle --max-depth 4')
run_cmd(bdtEx+ntpFolder+ntpTrain+' -o bdt40.pickle --max-depth 40')

## Loading BDTs
run_cmd(bdtEx+ntpFolder+ntpTrain+' -o l0hadron_bdt4_tm_train.root --load-bdt bdt4.pickle')
run_cmd(bdtEx+ntpFolder+ntpTrain+' -o l0hadron_bdt40_tm_train.root --load-bdt bdt40.pickle')
run_cmd(bdtEx+ntpFolder+ntpValid+' -o l0hadron_bdt4_tm_valid.root --load-bdt bdt4.pickle')
run_cmd(bdtEx+ntpFolder+ntpValid+' -o l0hadron_bdt40_tm_valid.root --load-bdt bdt40.pickle')
run_cmd(bdtEx+ntpFolder+ntpNtm+' -o l0hadron_bdt4_ntm.root --load-bdt bdt4.pickle')