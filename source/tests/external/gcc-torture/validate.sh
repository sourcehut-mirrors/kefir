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

set -e
set -o pipefail

torture_log="$1"
max_failed="$2"

grep -v "Interrupted system call" "$torture_log" | grep -i "fatal\|abort\|timeout\|segm" && exit 1 || true

failed=`sed -nr 's/^Failed tests: ([0-9]+).*$/\1/p' "$torture_log"`
echo -n "Number of failed tests $failed less of equal to $max_failed: "
[ "$failed" -le "$max_failed" ] \
    && echo "ok" \
    || (echo "fail"; exit 1)