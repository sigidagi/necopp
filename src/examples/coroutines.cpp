#include "necopp.hpp"
#include <iostream>
#include <chrono>
#include <fmt/format.h>

using namespace std::chrono_literals;
using namespace std::placeholders;

class Foo {
public:
    void ticker(int, void **) {
        while (1) {
            neco::sleep(1s);
            fmt::print("Foo tick\n");
        }
    }
    void tocker(int, void **) {
        while (1) {
            neco::sleep(2s);
            fmt::print("tock tock...\n");
        }
    }
};

int main_(int, char **) {
   
    Foo foo;
    neco::coroutine coro1 = std::bind(&Foo::ticker, &foo, _1, _2);
    neco::coroutine coro2 = std::bind(&Foo::tocker, &foo, _1, _2);
    // 1. 
    neco::result ret = neco::go(coro1)();
    if (ret != neco::result::OK) {
        fmt::print("Failed to start coroutine1\n");
        return (int)ret;
    }
    // 2. 
    ret = neco::go(coro2)();
    if (ret != neco::result::OK) {
        fmt::print("Failed to start coroutine2\n");
        return (int)ret;
    }
    
    // Keep the program alive for an hour.
    neco::sleep(1h);
    return 0;
}

int main(int argc, char* argv[]) {
    // run main program in Neco coroutine context
    return (int)neco::run(argc, argv, main_);
}
