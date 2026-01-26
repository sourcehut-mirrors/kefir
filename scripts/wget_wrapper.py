#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0
# 
# Copyright (C) 2020-2026  Jevgenijs Protopopovs
# 
# This file is part of Kefir project.
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
# # 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os
import sys
import subprocess
from shlex import quote

if __name__ == '__main__':
    WGET_CMD = os.environ.get("WGET_CMD")
    WGET_URL_MAP = os.environ.get("WGET_URL_MAP")

    if not WGET_CMD:
        WGET_CMD = 'wget'

    alts = dict()
    if WGET_URL_MAP and os.path.exists(WGET_URL_MAP):
        with open(WGET_URL_MAP) as f:
            for line in f:
                line = line.strip()
                if line and "|"  in line:
                    original, mapped = line.split("|", 1)
                    alts[original.strip()] = mapped.strip()

    args = (
        alts.get(arg, arg)
        for arg in sys.argv[1:]
    )
    os.execvp(WGET_CMD, [WGET_CMD, '--tries=0', '--retry-connrefused', '--waitretry=10', '--timeout=30', *args])
