#!/usr/bin/env bash

LOG_FILE="$1"

if grep 'of unexpected failures' "$LOG_FILE" >/dev/null; then
    exit 1
fi
