#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep '0 tests failed out of 130' "$LOG_FILE" >/dev/null; then
    exit 1
fi
