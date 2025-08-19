#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep 'PASSED (135 suites, 28899 tests run)' "$LOG_FILE" >/dev/null; then
    exit 1
fi
