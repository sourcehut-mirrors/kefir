#!/usr/bin/env bash

LOG_FILE="$1"

if grep -E 'fail:[ ]*[1-9]' "$LOG_FILE" >/dev/null; then
    exit 1
fi

