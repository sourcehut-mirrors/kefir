#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep "like you're in business" "$LOG_FILE" >/dev/null; then
    exit 1
fi
