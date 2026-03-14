#!/usr/bin/env bash

LIBWEBP_DIR="$1"

cd "$LIBWEBP_DIR"

./examples/dwebp examples/test.webp -ppm -o test.ppm
if [[ "$(sha256sum test.ppm | cut -f1 -d' ')" != "db448ba15096dd0941cacb7e7cc8f0bf5461226c423c330fa73d0591bd8ac980" ]]; then
    exit 1
fi