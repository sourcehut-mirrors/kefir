#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep '100% tests passed, 0 tests failed out of 19' "$LOG_FILE" >/dev/null; then
    exit 1
fi

