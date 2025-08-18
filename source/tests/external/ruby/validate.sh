#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep 'check succeeded' "$LOG_FILE" >/dev/null; then
    exit 1
fi
