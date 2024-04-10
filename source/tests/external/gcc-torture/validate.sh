#!/bin/sh

set -e
set -o pipefail

torture_log="$1"
max_failed="$2"

grep -v "Interrupted system call" "$torture_log" | grep -i "fatal\|abort\|timeout\|segm" && exit 1 || true

failed=`sed -nr 's/^Failed tests: ([0-9]+).*$/\1/p' "$torture_log"`
echo -n "Number of failed tests $failed less of equal to $max_failed: "
[ "$failed" -le "$max_failed" ] \
    && echo "ok" \
    || (echo "fail"; exit 1)