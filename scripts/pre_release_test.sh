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

set -e
set -o pipefail

export LC_ALL=C.UTF-8

SCRIPT_FILEPATH=`realpath "$0"`
SCRIPT_DIR=`dirname "$SCRIPT_FILEPATH"`
ROOT_DIR=`dirname "$SCRIPT_DIR"`

log () {
    local timestamp
    timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[$timestamp] $*"
}

fatal () {
    local timestamp
    timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[$timestamp] FATAL: $*"
    exit -1
}

if [[ "x$1" == "x" ]]; then
    fatal "Usage: $SCRIPT_FILEPATH outdir"
fi
OUTDIR=`realpath "$1"`/`date "+%s"`
shift 1

format_quote () {
    local prefix="$1"
    if [[ "x$prefix" == "x" ]]; then
        prefix="\t> "
    fi
    while IFS= read -r line; do
        echo -e "$prefix$line"
    done
}

print_kefir () {
    log "Kefir information:"
    echo "Git revision"
    git rev-parse HEAD 2>&1 | format_quote
    echo "Git status"
    git status | format_quote
    echo "$SCRIPT_FILEPATH sha256"
    sha256sum "$SCRIPT_FILEPATH" | format_quote
}

print_env () {
    log "Environment information:"
    echo "User"
    whoami | format_quote
    echo "Current working directory"
    pwd | format_quote
    echo "Environment variables"
    env | format_quote
    echo "Resource limits"
    ulimit -a | format_quote
    echo "CPU"
    lscpu | format_quote
    echo "RAM"
    free -h | format_quote
    echo "Disks"
    df -h | format_quote
    echo "Kernel"
    uname -a | format_quote
    echo "GCC version"
    gcc -v 2>&1 | format_quote
    echo "Clang version"
    clang -v 2>&1 | format_quote
    if command -v dpkg >/dev/null 2>&1; then
        echo "Package information"
        dpkg-query -W -f='${Package} ${Version}\n' | format_quote
    fi
}

own_test_suite () {
    mkdir -p "$OUTDIR/own"

    log "Own test suite with GCC host compiler"
    make test KEFIR_BIN_DIR="$ROOT_DIR/bin/own-gcc-host" USE_EXTENSION_SUPPORT=yes PROFILE=reldebug KEFIR_END2END_SELECTIVE_VALGRIND=yes CC=gcc -j$(nproc)  2>&1 | tee "$OUTDIR/own/gcc-host.log"

    log "Own test suite with GCC musl host compiler"
    make test  KEFIR_BIN_DIR="$ROOT_DIR/bin/own-gcc-musl-host" USE_SHARED=no CC=musl-gcc KEFIR_TEST_USE_MUSL=yes PROFILE=reldebug KEFIR_END2END_SELECTIVE_VALGRIND=yes -j$(nproc)  2>&1 | tee "$OUTDIR/own/gcc-musl-host.log"

    log "Own test suite with Clang host compiler"
    make test  KEFIR_BIN_DIR="$ROOT_DIR/bin/own-clang-host" USE_EXTENSION_SUPPORT=yes PROFILE=reldebug KEFIR_END2END_SELECTIVE_VALGRIND=yes CC=clang -j$(nproc) 2>&1 | tee "$OUTDIR/own/clang-host.log"

    log "Own test suite with Kefir host compiler"
    make KEFIR_BIN_DIR="$ROOT_DIR/bin/kefir-host-stage1" -j$(nproc)
    make install KEFIR_BIN_DIR="$ROOT_DIR/bin/kefir-host-stage1" prefix="$ROOT_DIR/bin/kefir-host-install"
    make test KEFIR_BIN_DIR="$ROOT_DIR/bin/kefir-host-stage2" USE_EXTENSION_SUPPORT=yes PROFILE=reldebug KEFIR_END2END_SELECTIVE_VALGRIND=yes CC="$ROOT_DIR/bin/kefir-host-install/bin/kefir" -j$(nproc) 2>&1 | tee "$OUTDIR/own/kefir-host.log"

    log "Own test suite with Kefir musl host compiler"
    make CC=musl-gcc USE_SHARED=no KEFIR_BIN_DIR="$ROOT_DIR/bin/kefir-musl-host-stage1" -j$(nproc)
    make install KEFIR_BIN_DIR="$ROOT_DIR/bin/kefir-musl-host-stage1" prefix="$ROOT_DIR/bin/kefir-musl-host-install"
    make test KEFIR_BIN_DIR="$ROOT_DIR/bin/kefir-musl-host-stage2" PROFILE=release KEFIR_TEST_USE_MUSL=yes USE_SHARED=no CC="$ROOT_DIR/bin/kefir-musl-host-install/bin/kefir" -j$(nproc) 2>&1 | tee "$OUTDIR/own/kefir-musl-host.log"
}

bootstrap_test () {
    mkdir -p "$OUTDIR/bootstrap"

    log "Reproducible bootstrap test with GNU As"
    make bootstrap_test KEFIR_BIN_DIR="$ROOT_DIR/bin/bootstrap-gas" BOOTSTRAP_EXTRA_CFLAGS="-O1 -g" -j$(nproc) 2>&1 | tee "$OUTDIR/bootstrap/bootstap-gas.log"

    log "Reproducible bootstrap test with Yasm"
    make bootstrap_test KEFIR_BIN_DIR="$ROOT_DIR/bin/bootstrap-yasm" BOOTSTRAP_EXTRA_CFLAGS="-O1 -masm=x86_64-yasm" AS=yasm -j$(nproc) 2>&1 | tee "$OUTDIR/bootstrap/bootstap-yasm.log"

    log "Reproducible bootstrap test with musl & GNU As"
    make bootstrap_test KEFIR_BIN_DIR="$ROOT_DIR/bin/bootstrap-musl-gas" BOOTSTRAP_EXTRA_CFLAGS="-O1 -g"  CC=musl-gcc USE_SHARED=no -j$(nproc) 2>&1 | tee "$OUTDIR/bootstrap/bootstap-musl-gas.log"

    log "Reproducible bootstrap test with musl & Yasm"
    make bootstrap_test KEFIR_BIN_DIR="$ROOT_DIR/bin/bootstrap-musl-yasm" BOOTSTRAP_EXTRA_CFLAGS="-O1 -masm=x86_64-yasm"  CC=musl-gcc USE_SHARED=no AS=yasm -j$(nproc) 2>&1 | tee "$OUTDIR/bootstrap/bootstap-musl-yasm.log"
}

portable_bootstrap_test () {
    mkdir -p "$OUTDIR/portable-bootstrap"

    log "Portable bootstrap test with GNU As"
    make portable_bootstrap KEFIR_BIN_DIR="$ROOT_DIR/bin/portable-bootstrap" -j$(nproc) 2>&1 | tee "$OUTDIR/portable-bootstrap.log"
}

print_make_deps () {
    local BIN_DIR="$1"
    shift 1
    local target deps
    for target in "$@"; do
        deps=$(make -qp -f "$ROOT_DIR/Makefile" KEFIR_BIN_DIR="$BIN_DIR" | awk -v tgt="$target" '$1 == tgt ":" {for (i=2; i<=NF; i++) if ($i != ".WAIT") print $i}')
        [[ -n "$deps" ]] && echo "$deps"
    done
}

external_test_suite () {
    mkdir -p "$OUTDIR/external"

    log "External tests"
    make KEFIR_BIN_DIR="$ROOT_DIR/bin/external" -j$(nproc)
    print_make_deps "$ROOT_DIR/bin/external" .EXTERNAL_TESTS_BASE_SUITE .EXTERNAL_TESTS_FAST_SUITE .EXTERNAL_TESTS_SLOW_SUITE | while read TEST_TARGET; do
        log "External test: $(basename $TEST_TARGET)"
        unbuffer make "$TEST_TARGET" KEFIR_BIN_DIR="$ROOT_DIR/bin/external" -j$(nproc) 2>&1 | tee "$OUTDIR/external/$(basename $TEST_TARGET).log"
    done

    log "Uncaptured external tests"
    unbuffer make .EXTERNAL_TESTS_SUITE KEFIR_BIN_DIR="$ROOT_DIR/bin/external" -j$(nproc) 2>&1 | tee "$OUTDIR/external/uncaptured.log"
}

webapp_build () {
    log "Building webapp"
    make KEFIR_BIN_DIR="$ROOT_DIR/bin/webapp" webapp -j$(nproc)
}

main () {
    log "Kefir pre-release test script"
    log "Writing all outputs to $OUTDIR"
    echo
    print_kefir
    echo
    print_env
    echo

    own_test_suite
    bootstrap_test
    portable_bootstrap_test
    external_test_suite
    webapp_build
    log "End of Kefir pre-release test"
}

cd "$ROOT_DIR"
make clean
mkdir -p "$OUTDIR"
main "$@" 2>&1 | tee "$OUTDIR/main.log"