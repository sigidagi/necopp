#include "necopp.hpp"
#include <iostream>
#include <memory>
#include <chrono>

using namespace std::chrono_literals;

struct Foo {
    int one;
    double two;
    std::shared_ptr<int> ptr = nullptr;
    const char* say;
};

int main_(int, char **) {
    
    // create a channel which can receive Foo objects.
    auto chFoo = neco::channel<Foo>();
    neco::go([&](int, void **) {

        std::cout << "Hello from coroutine\n";
        // Some heavy work here
        // ...
        // return Foo object as a result
        chFoo.sender.send({ 42, 3.14, std::make_shared<int>(66), "Hello" });
    })();
    
    Foo foo = chFoo.receiver.recv();
    std::cout << "received: " << foo.say << ", " << *foo.ptr << "\n";
    return 0;
}

int main(int argc, char* argv[]) {
    // run main program in Neco coroutine context
    return (int)neco::run(argc, argv, main_);
}

