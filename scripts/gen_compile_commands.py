#!/usr/bin/env python3
#
# SPDX-License-Identifier: GPL-3.0
#
# Copyright (C) 2020-2025  Jevgenijs Protopopovs
#
# This file is part of Kefir project.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
import sys
import os
import json
import pathlib
import argparse
from typing import Iterable

def scan_cmd_files(cmd_files: Iterable[pathlib.Path]):
    for cmd_filepath in cmd_files:
        with open(cmd_filepath) as cmd_file:
            root_dir = cmd_file.readline().strip()
            source_file = cmd_file.readline().strip()
            cmd_file.readline() # Skip target
            command = cmd_file.readline().strip()
            yield {
                'directory': root_dir,
                'file': source_file,
                'command': command
            }

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog=os.path.basename(__file__), description='compile_commands.json generator script for Kefir')
    parser.add_argument('--build-dir', type=str, required=True, help='Kefir build directory')
    task_group = parser.add_mutually_exclusive_group(required=True)
    task_group.add_argument('--cmd-files', action='store_true', default=False, help='List identified cmd files')
    task_group.add_argument('--compile-commands', action='store_true', default=False, help='Generate compile_commands.json database')
    args = parser.parse_args(sys.argv[1:])

    build_dir = pathlib.Path(args.build_dir)
    cmd_files = build_dir.glob('**/*.o.cmd')
    if args.cmd_files:
        for cmd_file in cmd_files:
            print(cmd_file.absolute())
    else:
        json.dump([
            entry
            for entry in scan_cmd_files(cmd_files)
        ], indent=2, fp=sys.stdout)
