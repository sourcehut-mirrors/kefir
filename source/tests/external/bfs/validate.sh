#!/usr/bin/env bash

LOG_FILE="$1"

if grep -i "fail" "$LOG_FILE"; then
    exit 1
fi
