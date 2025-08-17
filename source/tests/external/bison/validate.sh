#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep '707 tests were successful.' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep '69 tests were skipped.' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if grep 'fail' "$LOG_FILE" >/dev/null; then
    exit 1
fi
