#include "necopp.hpp"
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;
using namespace std::placeholders;

class Foo {
public:
    void ticker(int, void **) {
        while (1) {
            neco::sleep(1s);
            std::cout << "Foo tick\n";
        }
    }
    void tocker(int, void **) {
        while (1) {
            neco::sleep(2s);
            std::cout << "tock tock...\n";
        }
    }
};

int main_(int, char **) {
   
    Foo foo;
    neco::coroutine coro1 = std::bind(&Foo::ticker, &foo, _1, _2);
    neco::coroutine coro2 = std::bind(&Foo::tocker, &foo, _1, _2);
    // 1. 
    neco::Result ret = neco::go(coro1)();
    if (ret != neco::Result::OK) {
        std::cout << "Failed to start coroutine1\n";
        return (int)ret;
    }
    // 2. 
    ret = neco::go(coro2)();
    if (ret != neco::Result::OK) {
        std::cout << "Failed to start coroutine2\n";
        return (int)ret;
    }
    
    // Keep the program alive for an hour.
    neco::sleep(1h);
    return 0;
}

int main(int argc, char* argv[]) {
    // run main program in Neco coroutine context
    neco::Result ret = neco::run(argc, argv, main_);
    std::cout << "Main exit with code: " << (int)ret << std::endl;
}
