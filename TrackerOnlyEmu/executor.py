#!/usr/bin/env python3
#
# Author: Yipeng Sun
# License: BSD 2-clause
# Last Change: Tue Oct 26, 2021 at 04:24 PM +0200

from dataclasses import dataclass
from ROOT import RDataFrame
from ROOT.std import vector


@dataclass
class ExecDirective:
    op: str
    branch: str = None
    instruct: str = None
    keep: bool = False


def process_single_directive(attr, branch, instruct):
    if branch is not None:
        return attr(branch, instruct)
    return attr(instruct)


def process_directives(directives, init_frame):
    frames = []
    branches = []

    for d in directives:
        if not frames:
            prev_frame = init_frame
        else:
            prev_frame = frames[-1]

        cur_frame = process_single_directive(
            getattr(prev_frame, d.op), d.branch, d.instruct)
        frames.append(cur_frame)

        if d.keep:
            branches.append(d.branch)

    return frames, branches


def merge_vectors(*vecs, template='string'):
    base = vector(template)()

    for vec in vecs:
        for elem in vec:
            base.push_back(elem)

    return base
