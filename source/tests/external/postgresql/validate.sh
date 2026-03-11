#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep -E '^# All 230 tests passed.$' "$LOG_FILE" >/dev/null; then
    exit 1
fi
