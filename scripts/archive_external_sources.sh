#!/usr/bin/env bash
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

set -e

SCRIPT_DIR="$(dirname $(realpath $0))"
ROOT_DIR="$(realpath $SCRIPT_DIR/..)"
OUT_DIR="$1"

error () {
    local FMT="$1"
    shift 1
    printf "\e[;31m$FMT\n" $@ >&2
    exit -1
}

warn () {
    local FMT="$1"
    shift 1
    printf "\e[;93mWarning: $FMT\e[0m\n" $@ >&2
}

info () {
    local FMT="$1"
    shift 1
    printf "\e[;92m$FMT\e[0m\n" $@
}

archive_test () {
    local TEST_DIR="$1"
    local TEST_NAME="$(basename $1)"
    if [[ ! -f "$TEST_DIR/Makefile.mk" ]]; then
        warn "Unable to find Makefile.mk for external test %s" "$TEST_NAME"
        return
    fi

    make -pn -f "$TEST_DIR/Makefile.mk" | grep -E "KEFIR_EXTERNAL_TEST_[a-zA-Z0-9_]+_URL\s+:=" | while read -d $'\n' SOURCE_URL_DEFINITION; do
        local SOURCE_URL=`echo "$SOURCE_URL_DEFINITION" | awk -F':=' '{ print $2 }' | sed -re 's/\s*(.+)\s*/\1/g'`
        info "[%s] found URL: %s" "$TEST_NAME" "$SOURCE_URL"

        mkdir -p "$OUT_DIR/$TEST_NAME"
        (cd "$OUT_DIR/$TEST_NAME"; wget "$SOURCE_URL")
        echo "$TEST_NAME,$SOURCE_URL" >> "$OUT_DIR/index.csv"
    done
}

if [[ "x$OUT_DIR" == "x" ]]; then
    error "Usage: $0 out_directory"
else
    OUT_DIR="$(realpath $OUT_DIR)"
fi

if [[ ! -d "$OUT_DIR" ]]; then
    mkdir "$OUT_DIR"
fi

echo "Test,Source URL" > "$OUT_DIR/index.csv"
find "$ROOT_DIR/source/tests/external" -mindepth 1 -maxdepth 1 -type d -print0 | while read -d $'\0' TEST_DIR; do
    archive_test "$TEST_DIR"
done