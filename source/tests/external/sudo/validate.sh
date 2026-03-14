#!/usr/bin/env bash

LOG_FILE="$1"

if grep -oE '[0-9]+% success rate' "$LOG_FILE" | grep -v '100% success rate' >/dev/null; then
    exit 1
fi
