#include "necopp.hpp"
#include <iostream>
#include <chrono>
#include <fmt/format.h>

using namespace std::chrono_literals;

int main_(int, char **) {
    // create a generator of integers. 
    auto gen = neco::generator<int>([](int, void **) {
        // 
        for (int i = 0; i < 10; i++) {
            neco::sleep(300ms);
            neco::yield(i);
        }
    })();
    
    int i;
    while (gen.next(i) != neco::Result::CLOSED) {
        fmt::print("Got: {}", i);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    // run main program in Neco coroutine context
    neco::run(argc, argv, main_);
}

