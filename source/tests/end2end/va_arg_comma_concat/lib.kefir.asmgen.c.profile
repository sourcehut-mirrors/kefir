#!/usr/bin/env bash
KEFIR_CFLAGS="$KEFIR_CFLAGS -E -include $(dirname $SCRIPT)/include.h"