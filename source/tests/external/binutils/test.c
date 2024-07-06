#include <stdlib.h>

extern void greet(const char *);

int main(int argc, const char **argv) {
    if (argc > 1) {
        greet(argv[1]);
    }
    return EXIT_SUCCESS;
}