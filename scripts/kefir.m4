#!/usr/bin/env sh
#
# SPDX-License-Identifier: GPL-3.0
#
# Copyright (C) 2020-2023  Jevgenijs Protopopovs
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

SCRIPT_DIR="$(dirname $0)"
define(bindir, $SCRIPT_DIR/../bin)dnl
define(libdir, $SCRIPT_DIR/../lib)dnl
define(includedir, $SCRIPT_DIR/../include)dnl
define(sysconfdir, $SCRIPT_DIR/../etc)dnl

export LD_LIBRARY_PATH="libdir:$LD_LIBRARY_PATH"
export KEFIR_RTLIB="libdir/libkefirrt.a"
export KEFIR_RTINC="includedir/kefir/runtime"
if [ -f "sysconfdir/kefir.local" ]; then
    source "sysconfdir/kefir.local"
fi
exec "bindir/kefir-cc" "$@"