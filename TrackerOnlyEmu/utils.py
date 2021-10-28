#!/usr/bin/env python3
#
# Author: Yipeng Sun
# License: BSD 2-clause
# Last Change: Thu Oct 28, 2021 at 04:08 AM +0200

import builtins

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

def gen_output_dict(arr, names):
    return dict(zip(names, arr.T))


def slice_array(arr, right_idx):
    return arr[:, 0:right_idx]


def func_call_gen(func, params, particle=None):
    if particle is not None:
        params = [particle+'_'+p for p in params]

    return '{}({})'.format(func, ', '.join(params))
