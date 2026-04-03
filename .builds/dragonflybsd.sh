#!/usr/bin/env bash
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

# In absence of DragonflyBSD in Sourcehut build service, this script documents
# rough outline for kefir setup and validation on DragonflyBSD 6.4.2. Documented
# producedure is informative and is not guaranteed to be reproducible in the
# same sense other build manifests are. Extra packages/adjustments might be
# necessary to make it work. Nevertheless, the author was able to successfully
# execute full kefir BSD platform validation suite (as specified below).

set -ex

# Base dependencies & test suite
# pkg install bash binutils coreutils git gmake

export LC_ALL=en_US.UTF-8

# git clone -b develop https://git.sr.ht/~jprotopopov/kefir
# cd kefir

gmake clean
gmake PROFILE=reldebug -j
gmake test LD=/usr/local/bin/ld AS=/usr/local/bin/as
gmake install prefix=$HOME/kefir-install

# Extra validation & test suite
# pkg install python3 py311-sqlite3 wget

# GCC Torture suite
gmake torture_test LD=/usr/local/bin/ld AS=/usr/local/bin/as KEFIR_EXTERNAL_TEST_GCC_TORTURE_CFLAGS="-O1 -fPIC -pie -g -D__GNUC__=4 -D__GNUC_MINOR__=20 -D__GNUC_STDC_INLINE__=1 -include headers/bootstrap_include/dragonflybsd.h"

# c-testsuite
gmake bin/tests/external/c-testsuite.test.done LD=/usr/local/bin/ld AS=/usr/local/bin/as KEFIR_EXTERNAL_TEST_C_TESTSUITE_CFLAGS="-O1 -fPIC -pie -g -D__GNUC__=4 -D__GNUC_MINOR__=20 -D__GNUC_STDC_INLINE__=1 -include $PWD/headers/bootstrap_include/dragonflybsd.h"

# bootstrap
gmake bootstrap_test BOOTSTRAP_EXTRA_CFLAGS="-O1 -g" REALPATH=grealpath  AS=/usr/local/bin/as

# Lua
gmake bin/tests/external/lua-548.test.done LD=/usr/local/bin/ld AS=/usr/local/bin/as KEFIR_EXTERNAL_TEST_LUA_548_CFLAGS="-O1 -fPIC -pie -g -D__GNUC__=4 -D__GNUC_MINOR__=20 -D__GNUC_STDC_INLINE__=1 -include $PWD/headers/bootstrap_include/dragonflybsd.h" 

# self-host test suite run
gmake clean
gmake USE_EXTENSION_SUPPORT=yes CC=$HOME/kefir-install/bin/kefir  AS=/usr/local/bin/as EXTRA_CFLAGS="-D__GNUC__=4 -D__GNUC_MINOR__=20 -D__GNUC_STDC_INLINE__=1 -include headers/bootstrap_include/dragonflybsd.h" -j2
gmake USE_EXTENSION_SUPPORT=yes CC=$HOME/kefir-install/bin/kefir  AS=/usr/local/bin/as EXTRA_CFLAGS="-D__GNUC__=4 -D__GNUC_MINOR__=20 -D__GNUC_STDC_INLINE__=1 -include headers/bootstrap_include/dragonflybsd.h" test -j2