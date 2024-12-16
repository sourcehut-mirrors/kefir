#include <cstdlib>
#include <iostream>

int main(int argc, const char **argv) {
        std::cout << "Hello, " << argv[1] << " from " << argv[0] << std::endl;
        return EXIT_SUCCESS;
}
