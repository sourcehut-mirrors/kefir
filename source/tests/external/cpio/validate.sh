#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep "All 17 tests were successful." "$LOG_FILE" >/dev/null; then
    exit 1
fi
