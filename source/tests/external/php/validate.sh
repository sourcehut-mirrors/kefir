#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep -E "^Tests failed\s*:\s*0\s*\(\s*0.0%\s*\)\s*\(\s*0.0%\s*\)$" "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep -E "^Tests passed\s*:\s*13517" "$LOG_FILE" >/dev/null; then
    exit 1
fi
