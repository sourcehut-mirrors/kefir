#!/usr/bin/env bash

LOG_FILE="$1"

if ! grep '732182e0344b63d153f8e53ed657ea6bab4346e66a941ae52e21bd84f6896383[ ]*openPCells.svg' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep '613ea54080d74dba593f24991b415efb6afc001dc3bb58d673ea4a76b8cf0f5e[ ]*gilfoyle.gds' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep '8501aa9d90a7e801a5bbf4c2c836de7d5d9997873ab65e8c867c2e6be5c579a0[ ]*pineapple.gds' "$LOG_FILE" >/dev/null; then
    exit 1
fi

if ! grep '50437c0971edb7f77f1fab70d32ed022cd765ee1ca92f6bb078f542eb0fcec55[ ]*talentandsweat.gds' "$LOG_FILE" >/dev/null; then
    exit 1
fi
