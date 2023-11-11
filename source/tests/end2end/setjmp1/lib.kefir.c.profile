#!/usr/bin/env bash
KEFIR_CFLAGS="$KEFIR_CFLAGS --target naive-x86_64-host-none -I \"$(dirname $SCRIPT)/../../../headers/kefir/runtime/naive-amd64-sysv-gas\""
