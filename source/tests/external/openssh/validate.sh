#!/usr/bin/env bash

FILES_TESTS_LOG_FILE="$1"
T_EXEC_LOG_FILE="$2"
INTEROP_TESTS_LOG_FILE="$3"
EXTRA_TESTS_LOG_FILE="$4"
UNIT_LOG_FILE="$5"

if ! grep 'all file-tests passed' "$FILES_TESTS_LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep 'all t-exec passed' "$T_EXEC_LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep 'all interop-tests passed' "$INTEROP_TESTS_LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep 'all extra-tests passed' "$EXTRA_TESTS_LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep 'unit tests passed' "$UNIT_LOG_FILE" >/dev/null; then
    exit 1
fi
