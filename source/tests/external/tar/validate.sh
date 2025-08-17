#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep '230 tests were successful.' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep '14 tests were skipped.' "$LOG_FILE" >/dev/null; then
    exit 1
fi
