#!/usr/bin/env bash

TESTS_LOG_FILE="$1"

if ! grep -E '0 errors out of [0-9]+ tests' "$TESTS_LOG_FILE" >/dev/null; then
    exit 1
fi

