#!/usr/bin/env bash

GCC_SOURCE_DIR="$1"
TESTS_LIST="$2"

if [[ "x$GCC_SOURCE_DIR" == "x" ]] || [[ "x$TESTS_LIST" == "x" ]]; then
    echo "Usage: $0 gcc_source_dir tests_lst" >&2
    exit -1
fi

if [[ "x$CC" == "x" ]] then
    echo "Expected environment variable CC to contain compiler path" >&2
    exit -1
fi

if [[ "x$BUILD_TIMEOUT" == "x" ]] then
    BUILD_TIMEOUT=10
fi
if [[ "x$RUN_TIMEOUT" == "x" ]] then
    RUN_TIMEOUT=10
fi

tmpdir="$(mktemp -d)"

cleanup () {
    rm -rf "$tmpdir"
}

trap cleanup EXIT HUP INT QUIT PIPE TERM

run_test_case () {
    local filepath=`realpath "$GCC_SOURCE_DIR/$1"`
    if [[ ! -f "$GCC_SOURCE_DIR/$1" ]]; then
        echo "Unable to find test case $1 in $GCC_SOURCE_DIR" >&2
        exit -1
    fi

    echo -n "$1..."
    timeout "$BUILD_TIMEOUT" "$CC" $CFLAGS -o "$tmpdir/out" "$filepath"
    if [ $? -ne 0 ]; then
        echo "build failed"
        exit -1
    fi

    timeout "$RUN_TIMEOUT" "$tmpdir/out"
    if [ $? -ne 0 ]; then
        echo "run failed"
        exit -1
    fi

    echo "ok"
}

sed -nr "s/^(run)\s+(.*)$/\1:\2/p" "$TESTS_LIST" | while read LINE; do
    CMD=`echo "$LINE" | cut -f1 -d:`
    FILE=`echo "$LINE" | cut -f2- -d:`
    
    case "$CMD" in
        run)
            run_test_case "$FILE"
            ;;

        *)
            echo "Unknown command $CMD on line: $LINE" >&2
            exit -1
            ;;
    esac
done
