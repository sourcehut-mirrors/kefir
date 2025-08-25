#!/usr/bin/env bash

set -e

LOG_FILE="$1"

exec diff -u "$LOG_FILE" "$(dirname $(realpath $0))/expected.log"
