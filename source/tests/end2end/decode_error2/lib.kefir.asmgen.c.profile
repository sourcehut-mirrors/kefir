if [[ `uname` == "DragonFly" ]]; then
    export LC_ALL=en_US.UTF-8
else
    export LC_ALL=C.UTF-8
fi
KEFIR_CFLAGS="$KEFIR_CFLAGS -E"