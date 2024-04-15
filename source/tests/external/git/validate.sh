#!/usr/bin/env bash

LOG_FILE="$1"

if [[ "x$(cat $LOG_FILE | sed -nr 's/^failed[ ]*([0-9]+)$/\1/p')" != "x0"  ]]; then
    exit 1
fi
