#!/usr/bin/env bash

set -e

STAGE1="$1"
STAGE2="$2"

if [[ "x$REALPATH" == "x" ]]; then
    REALPATH="realpath"
fi

find "$STAGE1" -name "*.s" -print0 | while read -d $'\0' stage1file
do
    stage2file="$STAGE2/$($REALPATH --relative-to=$STAGE1 $stage1file)"
    diff -u "$stage1file" "$stage2file"
done

if [[ -f "$STAGE1/libkefir.so" ]]; then
    stage1_sha256=`sha256sum "$STAGE1/libkefir.so" | cut -f1 -d' '`
    stage2_sha256=`sha256sum "$STAGE2/libkefir.so" | cut -f1 -d' '`
    if [[ "$stage1_sha256" != "$stage2_sha256" ]]; then
        echo "libkefir.so sha256 mismatch"
        exit 1
    fi
fi

stage1_sha256=`sha256sum "$STAGE1/kefir" | cut -f1 -d' '`
stage2_sha256=`sha256sum "$STAGE2/kefir" | cut -f1 -d' '`
if [[ "$stage1_sha256" != "$stage2_sha256" ]]; then
    echo "kefir sha256 mismatch"
    exit 1
fi
