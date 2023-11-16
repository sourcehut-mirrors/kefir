#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0
# 
# Copyright (C) 2020-2023  Jevgenijs Protopopovs
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

# Compile Lua interpreter with Kefir and run Lua basic test suite

set -xe

KEFIRCC="$1"
BUILDDIR="$2"
LUA_VERSION="5.4.6"
SCRIPTDIR="$(realpath $(dirname $0))"

LUA_ARCHIVE="lua-$LUA_VERSION.tar.gz"
LUA_TESTS_ARCHIVE="lua-$LUA_VERSION-tests.tar.gz"
LUA_URL="https://www.lua.org/ftp/lua-$LUA_VERSION.tar.gz"
LUA_TESTS_URL="https://www.lua.org/tests/lua-$LUA_VERSION-tests.tar.gz"
LUA_DIR="lua-$LUA_VERSION"
LUA_TESTS_DIR="lua-$LUA_VERSION-tests"

rm -rf "$BUILDDIR"
mkdir "$BUILDDIR"
pushd "$BUILDDIR"

echo "Downloading Lua..."
wget -O "$LUA_ARCHIVE" "$LUA_URL"
wget -O "$LUA_TESTS_ARCHIVE" "$LUA_TESTS_URL"

echo "Unpacking Lua..."
tar xvf "$LUA_ARCHIVE"
tar xvf "$LUA_TESTS_ARCHIVE"

pushd "$LUA_DIR"

echo "Building Lua..."
mkdir -p bin
"$KEFIRCC" $KEFIR_CFLAGS -o bin/lua \
	$PWD/src/lapi.c \
	$PWD/src/lauxlib.c \
	$PWD/src/lbaselib.c \
	$PWD/src/lcode.c \
	$PWD/src/lcorolib.c \
	$PWD/src/lctype.c \
	$PWD/src/ldblib.c \
	$PWD/src/ldebug.c \
	$PWD/src/ldo.c \
	$PWD/src/ldump.c \
	$PWD/src/lfunc.c \
	$PWD/src/lgc.c \
	$PWD/src/linit.c \
	$PWD/src/liolib.c \
	$PWD/src/llex.c \
	$PWD/src/lmathlib.c \
	$PWD/src/lmem.c \
	$PWD/src/loadlib.c \
	$PWD/src/lobject.c \
	$PWD/src/lopcodes.c \
	$PWD/src/loslib.c \
	$PWD/src/lparser.c \
	$PWD/src/lstate.c \
	$PWD/src/lstring.c \
	$PWD/src/lstrlib.c \
	$PWD/src/ltable.c \
	$PWD/src/ltablib.c \
	$PWD/src/ltm.c \
	$PWD/src/lua.c \
	$PWD/src/lundump.c \
	$PWD/src/lutf8lib.c \
	$PWD/src/lvm.c \
	$PWD/src/lzio.c

popd

pushd "$LUA_TESTS_DIR"
if [ -f "$SCRIPTDIR/lua-$LUA_VERSION-tests.patch" ]; then
	echo "Patching Lua test suite..."
	# Patch is necessary due to musl locale quirks. The same issue arises with GCC-built Lua linked with musl
	patch < "$SCRIPTDIR/lua-$LUA_VERSION-tests.patch"
fi

echo "Running Lua basic test suite..."
"../$LUA_DIR/bin/lua" -e"_U=true" all.lua

echo "Successfully executed Lua basic test suite"
