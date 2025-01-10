#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep 'PASS: test/run-tests' "$LOG_FILE" >/dev/null; then
    exit 1
fi
