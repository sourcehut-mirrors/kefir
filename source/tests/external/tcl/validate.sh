#!/usr/bin/env bash

LOG_FILE="$1"

if [[ "x$(sed -nr 's/all\.tcl:.*Failed[[:space:]]*([0-9]+)/\1/p' $LOG_FILE | tail -n1)" != "x0"  ]]; then
    exit 1
fi
