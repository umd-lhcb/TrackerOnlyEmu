#!/usr/bin/env python3
#
# Author: Yipeng Sun
# Last Change: Thu Apr 15, 2021 at 09:15 PM +0200
# Stolen from: https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_L0HadronTOS.py

from math import floor


####################
# Helper functions #
####################

def find_binning_idx(x, x_binning):
    x_high, x_low, x_bins = x_binning
    cur_x_bin = floor(x / ((x_high-x_low) / x_bins))

    if cur_x_bin < 0:
        cur_x_bin = 0
    if cur_x_bin >= x_bins:
        cur_x_bin = x_bins - 1

    return cur_x_bin


########################
# Correction functions #
########################
# These are used to correct for 2-particle responses hitting on the
# calorimeters. The details are documented in LHCb-INT-2019-025

# NOTE: The responses from HCAL with a single pi is stored in a histogram as a
#       function of P, PT of the pi.
#       We use the responses as a smearing factor
def random_smearing(P, PT, realET, P_binning, PT_binning, P_PT_histos):
    # Choosing the P-PT bin/histo for this particle
    cur_P_bin = find_binning_idx(P, PT_binning)
    cur_PT_bin = find_binning_idx(PT, PT_binning)

    # Taking the right momentum region histo of calorimeter uncertainties
    cur_P_PT_histo = P_PT_histos[cur_P_bin][cur_PT_bin]
    if cur_P_PT_histo.GetEntries() == 0:
        return 0.0

    # Take a random value for the uncertainties from the histo
    rand = cur_P_PT_histo.GetRandom()

    # realET is the transverse energy from the tracker
    smearedET = realET * (1-rand)

    if smearedET < 0:
        smearedET = 0
    elif smearedET > 6100:  # Limitation from HCAL. HCAL is overloaded in this case
        smearedET = 6100

    return smearedET


def missing_fraction():
    pass
