void test(int x, ...) {
    __builtin_va_list args, args2;
    __builtin_va_start(args, x);
    __builtin_va_copy(args2, args);
    (void) __builtin_va_arg(args2, const long long **);
    __builtin_va_end(args);
}

void test2(void) {
    void *x = __builtin_alloca(100);
}
