#!/usr/bin/env bash
KEFIR_CFLAGS="$KEFIR_CFLAGS -E -P -include $(dirname $SCRIPT)/include.h"