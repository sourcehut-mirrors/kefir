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

if [[ "x$KEFIRCC" == "x" ]]; then
    echo "Define KEFIRCC environment variable"
    exit 1
fi

if [[ "x$TORTURE" == "x" ]]; then
    echo "Define TORTURE environment variable"
    exit 1
fi

KEFIRFLAGS="-include $(dirname $0)/torture.h --target host -g $KEFIR_EXTRAFLAGS"
SKIP_LIST="$(dirname $0)/torture.skip"
TIMEOUT=10
SKIPPED_TESTS=0
FAILED_TESTS=0
TOTAL_TESTS=0

ulimit -s 64000 # Stack size=64M

function is_test_skipped {
    grep "skip $1/$2" "$SKIP_LIST" >/dev/null
}

function run_test {(
    set -e
    if [[ "x$2" == "xexecute" ]]; then
      timeout $TIMEOUT $KEFIRCC $KEFIRFLAGS -o test.bin "$1"
      timeout $TIMEOUT ./test.bin
    else
      timeout $TIMEOUT $KEFIRCC $KEFIRFLAGS -S -o test.s "$1"
    fi
    rm -rf test.s test.bin
)}

function run_tests {
    for test_file in "$TORTURE/$1"/*.c ; do
      if is_test_skipped "$1" "$(basename $test_file)"; then
        result="Skip"
        SKIPPED_TESTS=$(( SKIPPED_TESTS + 1 ))
      else
        run_test "$test_file" "$1"
        if [ "$?" == "0" ] ; then
          result="Success"
        else
          result="Failure"
          FAILED_TESTS=$(( FAILED_TESTS + 1 ))
        fi
      fi
      TOTAL_TESTS=$(( TOTAL_TESTS + 1 ))
      echo "($FAILED_TESTS:$TOTAL_TESTS) $result $test_file"
    done
}

run_tests compile
run_tests execute
echo "Failed tests: $FAILED_TESTS; Skipped tests: $SKIPPED_TESTS; Total tests: $TOTAL_TESTS"

