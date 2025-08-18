#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep 'Successfully tested 2227 inputs' "$LOG_FILE"; then
    exit 1
fi
