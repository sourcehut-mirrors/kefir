ifdef(`extraargs', , `define(extraargs, -Wno-declare-decimal-support)')dnl
export KEFIR_EXTRAARGS="--target host-musl extraargs"

if [ -z "$KEFIR_MUSL_INCLUDE" ]; then
    export KEFIR_MUSL_INCLUDE="$SCRIPT_DIR/../include"
fi

if [ -z "$KEFIR_MUSL_LIB" ]; then
    export KEFIR_MUSL_LIB="$SCRIPT_DIR/../lib"
fi

if [ -z "$KEFIR_MUSL_DYNAMIC_LINKER" ]; then
    export KEFIR_MUSL_DYNAMIC_LINKER="$SCRIPT_DIR/../lib/ld-musl-x86_64.so.1"
fi
