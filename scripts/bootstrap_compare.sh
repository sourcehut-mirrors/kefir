#!/usr/bin/env bash

set -e

STAGE1="$1"
STAGE2="$2"

if [[ "x$REALPATH" == "x" ]]; then
    REALPATH="realpath"
fi

sha256_checksum () {
	sha256sum "$1" | cut -d' ' -f1
}


cksum_sha256_checksum () {
	cksum -a sha256  "$1" | cut -d' ' -f4
}

function do_sha256_checksum () {
    host=`uname | tr '[:upper:]' '[:lower:]'`
    case "$host" in
        "linux")
            sha256_checksum "$1"
            ;;

        "freebsd")
            sha256_checksum "$1"
            ;;

        "openbsd")
            cksum_sha256_checksum "$1"
            ;;

        "netbsd")
            cksum_sha256_checksum "$1"
            ;;
        
        *)
            echo "Unknown uname: $host"
            exit 1
            ;;
    esac
}

find "$STAGE1" -name "*.s" -print0 | while read -d $'\0' stage1file
do
    stage2file="$STAGE2/$($REALPATH --relative-to=$STAGE1 $stage1file)"
    diff -u "$stage1file" "$stage2file"
done

if [[ -f "$STAGE1/libkefir.so" ]]; then
    stage1_sha256=`do_sha256_checksum "$STAGE1/libkefir.so"`
    stage2_sha256=`do_sha256_checksum "$STAGE2/libkefir.so"`
    if [[ "$stage1_sha256" != "$stage2_sha256" ]]; then
        echo "libkefir.so sha256 mismatch"
        exit 1
    fi
fi

stage1_sha256=`do_sha256_checksum "$STAGE1/kefir"`
stage2_sha256=`do_sha256_checksum "$STAGE2/kefir"`
if [[ "$stage1_sha256" != "$stage2_sha256" ]]; then
    echo "kefir sha256 mismatch"
    exit 1
fi
