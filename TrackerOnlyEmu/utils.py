#!/usr/bin/env python3
#
# Author: Yipeng Sun
# License: BSD 2-clause
# Last Change: Fri Oct 29, 2021 at 01:37 AM +0200

import builtins
import numpy as np

from contextlib import contextmanager
from time import perf_counter


###########
# Generic #
###########

@contextmanager
def Timer():
    start = perf_counter()
    yield lambda: perf_counter() - start


def print_wrapper(msg, silent=False):
    if not silent:
        builtins.print(msg)


#############
# Run 2 RDX #
#############

def get_df_vars(frame, branches, transpose=True):
    if isinstance(branches, list):
        brs = np.array(list(frame.AsNumpy(columns=branches).values()))
        if transpose:
            return brs.T
        else:
            return brs

    return frame.AsNumpy(columns=[branches])[branches]


def gen_output_dict(arr, names):
    return dict(zip(names, arr.T))


def slice_array(arr, right_idx):
    return arr[:, 0:right_idx]


def func_call_gen(func, params, particle=None):
    if particle is not None:
        params = [particle+'_'+p for p in params]

    return '{}({})'.format(func, ', '.join(params))
