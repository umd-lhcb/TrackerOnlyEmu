#!/usr/bin/env python3
#
# Author: Yipeng Sun
# License: BSD 2-clause
# Last Change: Wed Mar 31, 2021 at 10:46 PM +0200

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
    else:
        return attr(instruct)


def process_directives(directives, init_frame):
    frames = []
    branches = vector('string')()

    for dir in directives:
        if not frames:
            prev_frame = init_frame
        else:
            prev_frame = frames[-1]

        cur_frame = process_single_directive(getattr(prev_frame, dir.op)(),
                                             dir.branch, dir.instruct)
        frames.append(cur_frame)

        if dir.keep:
            branches.push_back(dir.branch)

    return frames, branches
