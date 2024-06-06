#include "necopp.hpp"
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

int main_(int, char **) {
    
    neco::go([] (int, void**) {
        printf("Suspending coroutine\n");
        neco::suspend();
        printf("Coroutine resumed\n");
    })();

    for (int i = 0; i < 3; i++) {
        std::cout << i+1 << std::endl;
        neco::sleep(1s);
    }

    // Resume the suspended. The neco::lastid() returns the identifier for the
    // last coroutine started by the current coroutine.
    neco::resume(neco::lastid());
    return 0;
}

int main(int argc, char* argv[]) {
    return (int)neco::run(argc, argv, main_);
}
