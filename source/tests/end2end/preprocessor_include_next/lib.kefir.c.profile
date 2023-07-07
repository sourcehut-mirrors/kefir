#!/usr/bin/env bash
export LC_ALL=C
KEFIR_CFLAGS="$KEFIR_CFLAGS -I \"$(dirname $SRC_FILE)/headers\""
