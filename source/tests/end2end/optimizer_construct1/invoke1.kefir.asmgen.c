void test();
int test2();
void test3(int, int, int);
long test4(int, ...);
float test5(void);

void main() {
    test();
    test(1);
    test(1, 2, 3, 4, 5, 7);
    (void) test2();
    (void) test2("Hello, world!", 2 + 2 * 2);
    (void) test4(5, test2(), test3(1, 2, 3), test4(1, "HEY"), test5());

    (&test)("Test...");
    (void) (&test2)(2, 3, 4);
}
