#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep '# PASS:  3' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep '# FAIL:  0' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep '# ERROR: 0' "$LOG_FILE" >/dev/null; then
    exit 1
fi

