#include "necopp.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <fmt/format.h>

using namespace std::chrono_literals;

struct Foo {
    int one;
    double two;
    std::shared_ptr<int> ptr = nullptr;
    const char* say;

    std::string sayYou() const {
        return std::string(say);
    }
};

int main_(int, char **) {
    
    // create a channel witch can receive Foo objects.
    auto chFoo = neco::channel<Foo>();
    neco::go([&](int, void **) {

        fmt::print("Hello from coroutine\n");
        // Some heavy work here
        // ...
        // return Foo object as a result
        chFoo.sender.send({ 42, 3.14, std::make_shared<int>(66), "Hello" });
    })();
    
    Foo foo = chFoo.receiver.recv();
    fmt::print("Main received: '{}' two: '{}' ...\n", foo.say, *foo.ptr);
    return 0;
}

int main(int argc, char* argv[]) {
    // run main program in Neco coroutine context
    neco::run(argc, argv, main_);
}

