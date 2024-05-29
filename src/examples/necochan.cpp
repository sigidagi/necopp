#include "necopp.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <chrono>

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

int main_(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
   
    // create a channel witch can receive Foo objects.
    auto chFoo = neco::channel<Foo>();

    neco::go([&](int argc, void **argv) {
        (void)argc;
        (void)argv;
        // 
        chFoo.sender.send({ 42, 3.14, std::make_shared<int>(66), "Hello" });
    })();
    
    // copy received Foo object, otherwise it will be destroyed when neco::sleep will be called. 
    // TODO error handling
    Foo foo = chFoo.receiver.recv();

    neco::sleep(1s);
    std::cout << "Received say: " << foo.say << " two: " << *foo.ptr << std::endl;
    neco::sleep(1s);

    return 0;
}

int main(int argc, char* argv[]) {
    // run main program in Neco coroutine context
    neco::Result ret = neco::run(argc, argv, main_);
    std::cout << "Main exit with code: " << (int)ret << std::endl;
}

