#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep '0 unexpected' "$LOG_FILE" >/dev/null; then
    exit 1
fi
