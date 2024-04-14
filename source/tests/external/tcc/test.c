int printf(const char *, ...);

int main(int argc, const char **argv) {
    printf("Hello, %s from %s\n", argv[1], argv[0]);
    return 0;
}
