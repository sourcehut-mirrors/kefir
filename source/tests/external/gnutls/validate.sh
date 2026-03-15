#!/usr/bin/env bash

LOG_FILE="$1"

if grep -E '^# FAIL:[ ]*[0-9]$' "$LOG_FILE" | grep -vE '^# FAIL:[ ]*0$' >/dev/null; then
    exit 1
fi
