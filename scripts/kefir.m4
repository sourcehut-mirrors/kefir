#!/usr/bin/env sh
#
# SPDX-License-Identifier: GPL-3.0
#
# Copyright (C) 2020-2024  Jevgenijs Protopopovs
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

SCRIPT_DIR="$(realpath $(dirname $0))"
define(bindir, ../bin)dnl
define(libdir, ../lib)dnl
define(includedir, ../include)dnl
define(sysconfdir, ../etc)dnl

export LD_LIBRARY_PATH="$SCRIPT_DIR/libdir:$LD_LIBRARY_PATH"
if [ -z "$KEFIR_RTLIB" ]; then
    export KEFIR_RTLIB="$SCRIPT_DIR/libdir/libkefirrt.a"
fi
if [ -z "$KEFIR_RTINC" ]; then
    export KEFIR_RTINC="$SCRIPT_DIR/includedir/kefir/runtime"
fi
if [ -f "$SCRIPT_DIR/sysconfdir/kefir.local" ]; then
    . "$SCRIPT_DIR/sysconfdir/kefir.local"
fi
exec "$SCRIPT_DIR/bindir/kefir-cc" "$@"