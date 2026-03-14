#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep 'TOTAL: PASSED' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if grep 'TOTAL: FAIL' "$LOG_FILE" >/dev/null; then
    exit 1
fi
