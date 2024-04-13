#!/usr/bin/env bash

LOG_FILE="$1"
C_TESTSUTE_DIR="$2"

SUMMARY=`cat "$LOG_FILE" | "$C_TESTSUTE_DIR/scripts/tapsummary"`
echo "$SUMMARY"

if [[ "x$(echo "$SUMMARY" | sed -nr 's/fail\s*([0-9]+)/\1/p')" != "x0"  ]]; then
    exit 1
fi