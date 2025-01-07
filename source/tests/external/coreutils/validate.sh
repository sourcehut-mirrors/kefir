#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep -E '^# XFAIL: 0$' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep -E '^# FAIL:  0$' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep -E '^# ERROR: 0$' "$LOG_FILE" >/dev/null; then
    exit 1
fi
