#!/usr/bin/env sh
#
# SPDX-License-Identifier: GPL-3.0
#
# Copyright (C) 2020-2024  Jevgenijs Protopopovs
#
# This file is part of Kefir project.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

set -e

export LC_ALL=C

tempdir="$(mktemp -d)"

cleanup () {
    rm -rf "$tempdir"
}

trap cleanup EXIT HUP INT QUIT PIPE TERM

log () {
    echo $@ >&2
}

cc=cc
cflags=

host_cc=
host_os=
host_env=
clang_candidate_gcc=
include_path=
library_path=
dynamic_linker=

load_cc_from_env () {
    if [ "x$CC" != "x" ]; then
        cc="$CC"
    fi

    if [ "x$CFLAGS" != "x" ]; then
        cflags="$CFLAGS"
    fi
}

detect_host_compiler () {
    local res=""
    log -n "Detecting host C compiler... "

    cat > "$tempdir/test.c" <<EOF
#ifdef __clang__
"clang"
#elif defined(__KEFIRCC__)
"__KEFIRCC__"
#elif defined(__GNUC__)
"__GNUC__"
#endif
EOF

    res=`$cc $cflags -E "$tempdir/test.c"`
    if echo "$res" | grep clang >/dev/null; then
        host_cc='clang'
    elif echo "$res" | grep __KEFIRCC__ >/dev/null; then
        host_cc='kefir'
    elif echo "$res" | grep __GNUC__ >/dev/null; then
        host_cc='gcc'
    else
        log "failed to detect host compiler"
        exit -1
    fi

    log "$host_cc"
}

detect_host_os () {
    local os=""
    log -n "Detecting host OS... "

    os=`uname`
    if [ "$os" = 'Linux' ]; then
        host_os='linux'
    elif [ "$os" = 'FreeBSD' ]; then
        host_os='freebsd'
    elif [ "$os" = 'OpenBSD' ]; then
        host_os='openbsd'
    elif [ "$os" = 'NetBSD' ]; then
        host_os='netbsd'
    else
        log "failed to detect host OS"
        exit -1
    fi

    log "$host_os"
}

detect_musl () {
    echo | $cc $cflags -E -Wp,-v - 2>&1 >/dev/null | grep musl >/dev/null && echo 1 || echo 0
}

detect_host_env () {
    log -n "Detecting host environment... "

    if [ "$host_os" = "linux" ]; then
        if [ "$(detect_musl)" -eq 0 ]; then
            host_env="linux-gnu"
        else
            host_env="linux-musl"
        fi
    elif [ "$host_os" = "freebsd" ]; then
        host_env="freebsd-system"
    elif [ "$host_os" = "openbsd" ]; then
        host_env="openbsd-system"
    elif [ "$host_os" = "netbsd" ]; then
        host_env="netbsd-system"
    else
        log "failed to detect host environment"
        exit -1
    fi

    log "$host_env"
}

detect_clang_candidate_gcc () {
    clang_candidate_gcc=`$cc $cflags -v 2>&1 | sed -nr 's/Selected GCC installation:\s*(.*)/\1/p' | tr -d '\n'`

    if [ "x$clang_candidate_gcc" = "x" ]; then
        log "Clang compiler without a candidate GCC installation on Linux cannot be used as host environment provider due to header file incompatibility. Please use GCC for host environment configuration."
        exit -1
    else
        log "Using Clang selected candidate GCC installation: $clang_candidate_gcc"
    fi
}

detect_include_path () {
    local include_path1=""
    local include_path2=""

    log -n "Detecting include path... "
    
    case "$host_env" in
        "freebsd-system")
            include_path=`echo | $cc $cflags -E -Wp,-v - 2>&1 >/dev/null | grep -v clang | sed -nr 's/^ (.*)$/\1/p' | paste -sd ';' -`
            ;;

        "openbsd-system")
            include_path=`echo | $cc $cflags -E -Wp,-v - 2>&1 >/dev/null | sed -nr 's/^ (.*)$/\1/p' | paste -sd ';' -`
            ;;
        
        *)
            if [ "$host_cc" = "clang" ]; then
                include_path1=`echo -n "$clang_candidate_gcc/include;$clang_candidate_gcc/include-fixed"`
                include_path2=`echo | $cc $cflags -E -Wp,-v - 2>&1 >/dev/null | grep -v clang | sed -nr 's/^ (.*)$/\1/p' | paste -sd ';' -`
                include_path="$include_path1;$include_path2"
            else
                include_path=`echo | $cc $cflags -E -Wp,-v - 2>&1 >/dev/null | sed -nr 's/^ (.*)$/\1/p' | paste -sd ';' -`
            fi
            ;;
    esac

    log "$include_path"
}

detect_musl_libc () {
    $cc $cflags -Wl,--verbose </dev/null 2>&1 | grep -E '^.*/([^/]*musl[^/]*)/.*libc[^/]*\.(so|a)$' | head -n1
}

detect_library_path () {
    local library_path1=""
    local library_path2=""
    log -n "Detecting library path... "
    
    if [ "$host_env" = "linux-musl" ]; then
        library_path1=`dirname "$(detect_musl_libc)" | tr -d '\n'`
        library_path1="$library_path1;"
    fi

    if [ "$host_cc" = "clang" ]; then
        library_path2="$clang_candidate_gcc;$($cc $cflags -print-search-dirs | sed -nr 's/libraries: =(.*)/\1/p' | sed 's/:/;/g' | sed -nr 's/([^;]*clang[^;]*;?)//p')"
    else
        library_path2=`$cc $cflags -print-search-dirs | sed -nr 's/libraries: =(.*)/\1/p' | sed 's/:/;/g'`
    fi

    library_path="$library_path1$library_path2"
    log "$library_path"
}

detect_dynamic_linker () {
    log -n "Detecting dynamic linker... "

    cat > "$tempdir/test.c" <<EOF
#include <stdlib.h>

int main(void) {
    return EXIT_SUCCESS;
}
EOF
    
    $cc $cflags -o "$tempdir/test" "$tempdir/test.c"
    objcopy -O binary --only-section=.interp "$tempdir/test" "$tempdir/interp"
    dynamic_linker=`tr -d '\0' < "$tempdir/interp"`

    log "$dynamic_linker"
}

generate_header () {
    load_cc_from_env
    detect_host_compiler

    if [ "$host_cc" = "kefir" ]; then
        log -n "Generating header..."
        $cc $cflags --environment-header
        log "done"
        exit 0
    fi

    detect_host_os

    if [ "$host_os" = "linux" ] && [ "$host_cc" = "clang" ]; then
        detect_clang_candidate_gcc
    fi

    detect_host_env
    detect_include_path
    detect_library_path
    detect_dynamic_linker

    log -n "Generating header..."
    cat <<EOF
#define KEFIR_CONFIG_HOST_TARGET "x86_64-$host_env"
EOF

    case "$host_env" in
        "linux-gnu")
            cat <<EOF
#define KEFIR_CONFIG_HOST_AS "as"
#define KEFIR_CONFIG_HOST_LD "ld"
#define KEFIR_CONFIG_HOST_PLATFORM KEFIR_DRIVER_TARGET_PLATFORM_LINUX
#define KEFIR_CONFIG_HOST_VARIANT KEFIR_DRIVER_TARGET_VARIANT_GNU
#define KEFIR_CONFIG_HOST_LINUX_GNU_INCLUDE_PATH "$include_path"
#define KEFIR_CONFIG_HOST_LINUX_GNU_LIBRARY_PATH "$library_path"
#define KEFIR_CONFIG_HOST_LINUX_GNU_DYNAMIC_LINKER "$dynamic_linker"
EOF
            ;;

        "linux-musl")
            cat <<EOF
#define KEFIR_CONFIG_HOST_AS "as"
#define KEFIR_CONFIG_HOST_LD "ld"
#define KEFIR_CONFIG_HOST_PLATFORM KEFIR_DRIVER_TARGET_PLATFORM_LINUX
#define KEFIR_CONFIG_HOST_VARIANT KEFIR_DRIVER_TARGET_VARIANT_MUSL
#define KEFIR_CONFIG_HOST_LINUX_MUSL_INCLUDE_PATH "$include_path"
#define KEFIR_CONFIG_HOST_LINUX_MUSL_LIBRARY_PATH "$library_path"
#define KEFIR_CONFIG_HOST_LINUX_MUSL_DYNAMIC_LINKER "$dynamic_linker"
EOF
            ;;

        "freebsd-system")
            cat <<EOF
#define KEFIR_CONFIG_HOST_AS "as"
#define KEFIR_CONFIG_HOST_LD "ld"
#define KEFIR_CONFIG_HOST_PLATFORM KEFIR_DRIVER_TARGET_PLATFORM_FREEBSD
#define KEFIR_CONFIG_HOST_VARIANT KEFIR_DRIVER_TARGET_VARIANT_SYSTEM
#define KEFIR_CONFIG_HOST_FREEBSD_SYSTEM_INCLUDE_PATH "$include_path"
#define KEFIR_CONFIG_HOST_FREEBSD_SYSTEM_LIBRARY_PATH "$library_path"
#define KEFIR_CONFIG_HOST_FREEBSD_SYSTEM_DYNAMIC_LINKER "$dynamic_linker"
EOF
            ;;

        "openbsd-system")
            cat <<EOF
#define KEFIR_CONFIG_HOST_AS "gas"
#define KEFIR_CONFIG_HOST_LD "ld"
#define KEFIR_CONFIG_HOST_PLATFORM KEFIR_DRIVER_TARGET_PLATFORM_OPENBSD
#define KEFIR_CONFIG_HOST_VARIANT KEFIR_DRIVER_TARGET_VARIANT_SYSTEM
#define KEFIR_CONFIG_HOST_OPENBSD_SYSTEM_INCLUDE_PATH "$include_path"
#define KEFIR_CONFIG_HOST_OPENBSD_SYSTEM_LIBRARY_PATH "$library_path"
#define KEFIR_CONFIG_HOST_OPENBSD_SYSTEM_DYNAMIC_LINKER "$dynamic_linker"
EOF
            ;;

        "netbsd-system")
            cat <<EOF
#define KEFIR_CONFIG_HOST_AS "as"
#define KEFIR_CONFIG_HOST_LD "ld"
#define KEFIR_CONFIG_HOST_PLATFORM KEFIR_DRIVER_TARGET_PLATFORM_NETBSD
#define KEFIR_CONFIG_HOST_VARIANT KEFIR_DRIVER_TARGET_VARIANT_SYSTEM
#define KEFIR_CONFIG_HOST_NETBSD_SYSTEM_INCLUDE_PATH "$include_path"
#define KEFIR_CONFIG_HOST_NETBSD_SYSTEM_LIBRARY_PATH "$library_path"
#define KEFIR_CONFIG_HOST_NETBSD_SYSTEM_DYNAMIC_LINKER "$dynamic_linker"
EOF
            ;;
    esac

    log "done"
}

generate_environment () {
    load_cc_from_env
    detect_host_compiler

    if [ "$host_cc" = "kefir" ]; then
        log -n "Generating environment..."
        $cc $cflags --environment-info | grep -v "WORKDIR\|RTINC" | sed -e "s/^/export /g"
        log "done"
        exit 0
    fi

    detect_host_os

    if [ "$host_os" = "linux" ] && [ "$host_cc" = "clang" ]; then
        detect_clang_candidate_gcc
    fi

    detect_host_env
    detect_include_path
    detect_library_path
    detect_dynamic_linker

    log -n "Generating environment..."
    cat <<EOF
export KEFIR_TARGET="x86_64-$host_env"
EOF

    case "$host_env" in
        "linux-gnu")
            cat <<EOF
export KEFIR_AS="as"
export KEFIR_LD="ld"
export KEFIR_GNU_INCLUDE="$include_path"
export KEFIR_GNU_LIB="$library_path"
export KEFIR_GNU_DYNAMIC_LINKER="$dynamic_linker"
EOF
            ;;

        "linux-musl")
            cat <<EOF
export KEFIR_AS="as"
export KEFIR_LD="ld"
export KEFIR_MUSL_INCLUDE="$include_path"
export KEFIR_MUSL_LIB="$library_path"
export KEFIR_MUSL_DYNAMIC_LINKER="$dynamic_linker"
EOF
            ;;

        "freebsd-system")
            cat <<EOF
export KEFIR_AS="as"
export KEFIR_LD="ld"
export KEFIR_FREEBSD_INCLUDE="$include_path"
export KEFIR_FREEBSD_LIB="$library_path"
export KEFIR_FREEBSD_DYNAMIC_LINKER="$dynamic_linker"
EOF
            ;;

        "openbsd-system")
            cat <<EOF
export KEFIR_AS="gas"
export KEFIR_LD="ld"
export KEFIR_OPENBSD_INCLUDE="$include_path"
export KEFIR_OPENBSD_LIB="$library_path"
export KEFIR_OPENBSD_DYNAMIC_LINKER="$dynamic_linker"
EOF
            ;;

        "netbsd-system")
            cat <<EOF
export KEFIR_AS="as"
export KEFIR_LD="ld"
export KEFIR_NETBSD_INCLUDE="$include_path"
export KEFIR_NETBSD_LIB="$library_path"
export KEFIR_NETBSD_DYNAMIC_LINKER="$dynamic_linker"
EOF
            ;;
    esac

    log "done"
}

for ARG in "$@"; do
    case "$ARG" in
        "--header")
            generate_header
            exit 0
            ;;

        "--environment")
            generate_environment
            exit 0
            ;;

        "--help")
            echo "$0 is used to detect environment (include paths, library paths, dynamic linker, suitable kefir target) on the host and" \
                 "output it as a C header or shell environment into stdout."
            echo
            echo "Options:"
            echo "    --header      Generate C header file"
            echo "    --environment Generate shell exports"
            echo "    --help        Show this help"
            echo
            echo "Environment variables:"
            echo "    CC     C compiler to use for detection"
            echo "    CFLAGS C compilation flags to use for detection"
            echo
            echo "Author: Jevgenijs Protopopovs"
            echo "License: GNU GPLv3"
            exit 0
            ;;

        *)
            log "Unknown option $ARG. Usage: $0 --header | --environment | --help"
            exit -1
            ;;
    esac

    log "Usage: $0 --header | --environment | --help"
done