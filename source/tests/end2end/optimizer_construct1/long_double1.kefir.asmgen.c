void test() {
    (void) (3.14L);
    (void) (1.0L + 2.0L);
    (void) (2.0L * 3.0L);
    (void) (100.0L - 5.4L);
    (void) (324.2L / 100.0L);
    (void) (-(1982.1L));
}

void test2() {
    long double x = 4.281l;
}

void test3() {
    if (1.0L)
        ;
    (void) (int) 1.0L;
    (void) (float) 1.0L;
    (void) (double) 1.0L;
}

void test4() {
    (void) (long double) 1l;
    (void) (long double) 1ul;
    (void) (long double) 1.0f;
    (void) (long double) 1.0;
}
