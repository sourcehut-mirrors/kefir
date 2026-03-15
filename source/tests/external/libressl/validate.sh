#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep -E '^# PASS:  135$' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep -E '^# FAIL:  0$' "$LOG_FILE" >/dev/null; then
    exit 1
fi
