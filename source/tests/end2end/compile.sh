#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0
# 
# Copyright (C) 2020-2024  Jevgenijs Protopopovs
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

SCRIPT="$0"
KEFIR_BIN_DIR="$1"
SRC_FILE="$2"
DST_FILE="$3"
if [[ "x$DST_FILE" == "x" ]]; then
    echo "Usage: $0 bin_dir source_file destination_file"
    exit -1
fi

KEFIRCC="$KEFIR_BIN_DIR/kefir"
export LD_LIBRARY_PATH="$KEFIR_BIN_DIR/libs"
export KEFIR_RTINC="$(dirname $SCRIPT)/../../../headers/kefir/runtime"

KEFIR_CFLAGS=" --target host-none -fPIC -pie -I \"$(dirname $SRC_FILE)\" -I \"$(dirname $SCRIPT)/../../../headers/kefir/runtime/common\""

if [[ "x$ASMGEN" == "xyes" ]]; then
    KEFIR_CFLAGS="$KEFIR_CFLAGS -Wno-codegen-emulated-tls -S"
else
    KEFIR_CFLAGS="$KEFIR_CFLAGS -c"
fi

if [[ -f "$SRC_FILE.profile" ]]; then
    source "$SRC_FILE.profile"
fi

set -e

if [[ "x$USE_VALGRIND" == "xyes" ]]; then
    eval valgrind $VALGRIND_TEST_OPTIONS "$KEFIRCC" $KEFIR_CFLAGS "$SRC_FILE" -o "$DST_FILE"
else
    eval "$KEFIRCC" $KEFIR_CFLAGS "$SRC_FILE" -o "$DST_FILE"
fi