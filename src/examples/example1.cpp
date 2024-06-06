#include "necopp.hpp"
#include <iostream>

int main_(int, char **) {
    std::cout << "Hello, coroutine!\n";
    return 0;
}

int main(int argc, char* argv[]) {
    // run main program in Neco coroutine context
    return (int)neco::run(argc, argv, main_);
}
