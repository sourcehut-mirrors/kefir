#!/usr/bin/env bash

LOG_FILE="$1"

if [[ x`grep -E '^# XFAIL: 0$' "$LOG_FILE" | wc -l` != "x8" ]]; then
    exit 1
fi

if [[ x`grep -E '^# FAIL:  0$' "$LOG_FILE" | wc -l` != "x8" ]]; then
    exit 1
fi

if [[ x`grep -E '^# ERROR: 0$' "$LOG_FILE" | wc -l` != "x8" ]]; then
    exit 1
fi
