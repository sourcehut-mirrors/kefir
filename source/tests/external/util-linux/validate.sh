#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep -E 'All 337 tests PASSED' "$LOG_FILE" >/dev/null; then
    exit 1
fi
