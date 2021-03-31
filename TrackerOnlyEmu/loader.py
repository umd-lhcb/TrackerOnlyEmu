#!/usr/bin/env python3
#
# Author: Yipeng Sun
# License: BSD 2-clause
# Last Change: Wed Mar 31, 2021 at 04:11 PM +0200

from os import path
from ROOT import gInterpreter


def load_cpp(filepath, current_file_path=__file__):
    if filepath.startswith('<') and filepath.endswith('>'):
        filepath = path.join(path.abspath(path.dirname(current_file_path)),
                             filepath[1:-1])

    # Now load the CPP file in ROOT
    with open(filepath, 'r') as f:
        content = f.read()

    gInterpreter.Declare(content)
