#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep -E '^[ ]+0 tests failed$' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep -E '^[ ]+0 unexpected errors$' "$LOG_FILE" >/dev/null; then
    exit 1
fi
