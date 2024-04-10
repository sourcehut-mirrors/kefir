#!/bin/sh

set -e

sha256_checksum () {
	local checksum=`sha256sum "$1" | cut -d' ' -f1`
    if [ "$checksum" == "$2" ]; then
        echo "$(basename $1) sha256 check: ok"
        exit 0
    else
        echo "$(basename $1) sha256 check: fail"
        exit 1
    fi
}


cksum_sha256_checksum () {
	local checksum=`cksum -a sha256  "$1" | cut -d' ' -f4`
    if [ "$checksum" == "$2" ]; then
        echo "$(basename $1) sha256 check: ok"
        exit 0
    else
        echo "$(basename $1) sha256 check: fail"
        exit 1
    fi
}

host=`uname | tr '[:upper:]' '[:lower:]'`
case "$host" in
    "linux")
        sha256_checksum "$1" "$2"
        ;;

    "freebsd")
        sha256_checksum "$1" "$2"
        ;;

    "openbsd")
        cksum_sha256_checksum "$1" "$2"
        ;;

    "netbsd")
        cksum_sha256_checksum "$1" "$2"
        ;;
    
    *)
        echo "Unknown uname: $host"
        exit 1
        ;;
esac
