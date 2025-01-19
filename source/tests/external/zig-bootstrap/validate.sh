#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep -E 'pub const zig_version_string = "0.14.0-dev.bootstrap";' "$LOG_FILE" >/dev/null; then
    exit 1
fi


