#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep -E "Suites.*Failed\s+0[^0-9]" "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep -E "Tests.*Failed\s+0[^0-9]" "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep -E "Asserts.*Failed\s+0[^0-9]" "$LOG_FILE" >/dev/null; then
    exit 1
fi
