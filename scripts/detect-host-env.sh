#!/usr/bin/env bash

set -e

export LC_ALL=C

tempdir="$(mktemp -d)"

cleanup () {
    rm -rf "$tempdir"
}

trap cleanup EXIT HUP INT QUIT PIPE TERM

cc=cc
cflags=
outfile="$1"

if [ "x$outfile" = "x" ]; then
    echo "Usage: $0 output_file"
    exit -1
fi

host_cc=
host_os=
host_env=
include_path=
library_path=
dynamic_linker=

detect_host_compiler () {
    cat > "$tempdir/test.c" <<EOF
#ifdef __clang__
"clang"
#elif defined(__KEFIRCC__)
"__KEFIRCC__"
#elif defined(__GNUC__)
"__GNUC__"
#endif
EOF

    local res=`$cc $cflags -E "$tempdir/test.c"`
    if echo "$res" | grep clang >/dev/null; then
        echo 'clang'
    elif echo "$res" | grep __KEFIRCC__ >/dev/null; then
        echo 'kefir'
    elif echo "$res" | grep __GNUC__ >/dev/null; then
        echo 'gcc'
    else
        echo 'unknown'
    fi
}

detect_host_os () {
    local os=`uname`
    if [ "$os" = 'Linux' ]; then
        echo 'linux'
    elif [ "$os" = 'FreeBSD' ]; then
        echo 'freebsd'
    elif [ "$os" = 'OpenBSD' ]; then
        echo 'openbsd'
    elif [ "$os" = 'NetBSD' ]; then
        echo 'netbsd'
    else
        echo 'unknown'
    fi
}

detect_musl () {
    echo | $cc $cflags -E -Wp,-v - 2>&1 >/dev/null | grep musl >/dev/null && echo 1 || echo 0
}

detect_host_env () {
    if [ "$host_os" = "linux" ]; then
        if [ "$(detect_musl)" -eq 0 ]; then
            echo "linux-gnu"
        else
            echo "linux-musl"
        fi
    elif [ "$host_os" = "freebsd" ]; then
        echo "freebsd-system"
    elif [ "$host_os" = "openbsd" ]; then
        echo "openbsd-system"
    elif [ "$host_os" = "netbsd" ]; then
        echo "netbsd-system"
    else
        echo "unknown"
    fi
}

detect_include_path () {
    case "$host_env" in
        "freebsd-system")
            echo | $cc $cflags -E -Wp,-v - 2>&1 >/dev/null | grep -v clang | sed -nr 's/^ (.*)$/\1/p' | paste -sd ';' -
            ;;

        "openbsd-system")
            echo | $cc $cflags -E -Wp,-v - 2>&1 >/dev/null | grep -v clang | sed -nr 's/^ (.*)$/\1/p' | paste -sd ';' -
            ;;
        
        *)
            echo | $cc $cflags -E -Wp,-v - 2>&1 >/dev/null | sed -nr 's/^ (.*)$/\1/p' | paste -sd ';' -
            ;;
    esac
}

detect_musl_libc () {
    $cc $cflags -Wl,--verbose </dev/null 2>&1 | grep -E '^.*/([^/]*musl[^/]*)/.*libc[^/]*\.(so|a)$' | head -n1
}

detect_library_path () {
    if [ "$host_env" = "linux-musl" ]; then
        dirname $(detect_musl_libc) | tr -d '\n'
        echo -n ';'
    fi
    $cc $cflags -print-search-dirs | sed -nr 's/libraries: =(.*)/\1/p' | sed 's/:/;/g'
}

detect_dynamic_linker () {
    cat > "$tempdir/test.c" <<EOF
#include <stdlib.h>

int main(void) {
    return EXIT_SUCCESS;
}
EOF
    
    $cc $cflags -o "$tempdir/test" "$tempdir/test.c"
    objcopy -O binary --only-section=.interp "$tempdir/test" "$tempdir/interp"
    tr -d '\0' < "$tempdir/interp"
}

if [ "x$CC" != "x" ]; then
    cc="$CC"
fi

if [ "x$CFLAGS" != "x" ]; then
    cflags="$CFLAGS"
fi

echo -n "Detecting host C compiler... "
host_cc=`detect_host_compiler`
echo "$host_cc"

if [ "$host_cc" = "kefir" ]; then
    echo -n "Generating $outfile... "
    $cc $cflags --compiler-host-environment-header > "$outfile"
    echo "done"
    exit 0
fi

echo -n "Detecting host OS... "
host_os=`detect_host_os`
echo "$host_os"

if [ "$host_os" = "linux" ] && [ "$host_cc" = "clang" ]; then
    echo "Clang compiler on Linux cannot be used as host environment due to header file incompatibility. Please use GCC for host environment configuration."
    exit -1
fi

echo -n "Detecting host environment... "
host_env=`detect_host_env`
echo "$host_env"
if [ "$host_env" = "unknown" ]; then
    echo "Failed to detect host environment"
    exit -1
fi

echo -n "Detecting include path... "
include_path=`detect_include_path`
echo "$include_path"

echo -n "Detecting library path... "
library_path=`detect_library_path`
echo "$library_path"

echo -n "Detecting dynamic linker... "
dynamic_linker=`detect_dynamic_linker`
echo "$dynamic_linker"

echo -n "Generating $outfile... "
cat >"$outfile" <<EOF
#define KEFIR_CONFIG_HOST_ENVIRONMENT "$host_env"
EOF

case "$host_env" in
    "linux-gnu")
        cat >>"$outfile" <<EOF
#define KEFIR_CONFIG_HOST_PLATFORM KEFIR_DRIVER_TARGET_PLATFORM_LINUX
#define KEFIR_CONFIG_HOST_VARIANT KEFIR_DRIVER_TARGET_VARIANT_GNU
#define KEFIR_CONFIG_HOST_LINUX_GNU_INCLUDE_PATH "$include_path"
#define KEFIR_CONFIG_HOST_LINUX_GNU_LIBRARY_PATH "$library_path"
#define KEFIR_CONFIG_HOST_LINUX_GNU_DYNAMIC_LINKER "$dynamic_linker"
EOF
        ;;

    "linux-musl")
        cat >>"$outfile" <<EOF
#define KEFIR_CONFIG_HOST_PLATFORM KEFIR_DRIVER_TARGET_PLATFORM_LINUX
#define KEFIR_CONFIG_HOST_VARIANT KEFIR_DRIVER_TARGET_VARIANT_MUSL
#define KEFIR_CONFIG_HOST_LINUX_MUSL_INCLUDE_PATH "$include_path"
#define KEFIR_CONFIG_HOST_LINUX_MUSL_LIBRARY_PATH "$library_path"
#define KEFIR_CONFIG_HOST_LINUX_MUSL_DYNAMIC_LINKER "$dynamic_linker"
EOF
        ;;

    "freebsd-system")
        cat >>"$outfile" <<EOF
#define KEFIR_CONFIG_HOST_PLATFORM KEFIR_DRIVER_TARGET_PLATFORM_FREEBSD
#define KEFIR_CONFIG_HOST_VARIANT KEFIR_DRIVER_TARGET_VARIANT_SYSTEM
#define KEFIR_CONFIG_HOST_FREEBSD_SYSTEM_INCLUDE_PATH "$include_path"
#define KEFIR_CONFIG_HOST_FREEBSD_SYSTEM_LIBRARY_PATH "$library_path"
#define KEFIR_CONFIG_HOST_FREEBSD_SYSTEM_DYNAMIC_LINKER "$dynamic_linker"
EOF
        ;;

    "openbsd-system")
        cat >>"$outfile" <<EOF
#define KEFIR_CONFIG_HOST_PLATFORM KEFIR_DRIVER_TARGET_PLATFORM_OPENBSD
#define KEFIR_CONFIG_HOST_VARIANT KEFIR_DRIVER_TARGET_VARIANT_SYSTEM
#define KEFIR_CONFIG_HOST_OPENBSD_SYSTEM_INCLUDE_PATH "$include_path"
#define KEFIR_CONFIG_HOST_OPENBSD_SYSTEM_LIBRARY_PATH "$library_path"
#define KEFIR_CONFIG_HOST_OPENBSD_SYSTEM_DYNAMIC_LINKER "$dynamic_linker"
EOF
        ;;

    "netbsd-system")
        cat >>"$outfile" <<EOF
#define KEFIR_CONFIG_HOST_PLATFORM KEFIR_DRIVER_TARGET_PLATFORM_NETBSD
#define KEFIR_CONFIG_HOST_VARIANT KEFIR_DRIVER_TARGET_VARIANT_SYSTEM
#define KEFIR_CONFIG_HOST_NETBSD_SYSTEM_INCLUDE_PATH "$include_path"
#define KEFIR_CONFIG_HOST_NETBSD_SYSTEM_LIBRARY_PATH "$library_path"
#define KEFIR_CONFIG_HOST_NETBSD_SYSTEM_DYNAMIC_LINKER "$dynamic_linker"
EOF
        ;;
esac
echo "done"
