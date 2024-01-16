#!/usr/bin/env sh

if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    printf "r%s.%s" "$(git rev-list --count HEAD 2>/dev/null)" "$(git rev-parse HEAD  2>/dev/null)"
fi