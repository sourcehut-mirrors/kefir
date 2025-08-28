#!/usr/bin/env bash
export LC_ALL=C
KEFIR_CFLAGS="$KEFIR_CFLAGS -isystem \"$(dirname $SRC_FILE)/headers\" -isystem \"$(dirname $SRC_FILE)/headers2\" -E"
