#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0
# 
# Copyright (C) 2020-2025  Jevgenijs Protopopovs
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

set -eo pipefail

SCRIPT_DIR="$(dirname `realpath $0`)"
OUT_FILE="$1"

TMP_FILE="$OUT_FILE.tmp"

"$SCRIPT_DIR/run_gcc_torture_suite.sh" 2>&1 | tee -i "$TMP_FILE"
"$SCRIPT_DIR/validate.sh" "$TMP_FILE"

mv "$TMP_FILE" "$OUT_FILE"