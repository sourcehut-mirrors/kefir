#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep 'OK: 1626' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep 'OK: 100' "$LOG_FILE" >/dev/null; then
    exit 1
fi
