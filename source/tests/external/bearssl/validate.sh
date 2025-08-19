#!/usr/bin/env bash

LOG_FILE="$1"

if grep -i 'fail' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if grep -i 'err' "$LOG_FILE" >/dev/null; then
    exit 1
fi
