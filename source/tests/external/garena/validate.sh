#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep 'Finished. Passed 16/16 tests.' "$LOG_FILE" >/dev/null; then
    exit 1
fi
