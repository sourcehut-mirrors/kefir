#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep '61 successful test scripts, 0 failures, 3 skipped' "$LOG_FILE" >/dev/null; then
    exit 1
fi
