#!/usr/bin/env python3
#
# Author: Yipeng Sun
# License: BSD 2-clause
# Last Change: Sun Apr 11, 2021 at 11:36 PM +0200

from os import path
from ROOT import gInterpreter


def load_file(filepath, current_file_path=__file__):
    if filepath.startswith('<') and filepath.endswith('>'):
        filepath = path.join(path.abspath(path.dirname(current_file_path)),
                             filepath[1:-1])

    return filepath


def load_cpp(filepath, current_file_path=__file__):
    filepath = load_file(filepath, current_file_path)

    # Now load the CPP file in ROOT
    with open(filepath, 'r') as f:
        content = f.read()

    gInterpreter.Declare(content)
