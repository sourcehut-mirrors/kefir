#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep -E '^finished [0-9]+ tests, [0-9]+ expected fail, 0 fail, [0-9]+ skipped$' "$LOG_FILE" >/dev/null; then
    exit 1
fi
