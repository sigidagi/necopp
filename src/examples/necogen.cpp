#include "necopp.hpp"
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

int main_(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
   
    // create a generator of integers. 
    auto gen = neco::generator<int>([](int argc, void **argv) {
        (void)argc;
        (void)argv;
        
        for (int i = 0; i < 10; i++) {
            neco::sleep(300ms);
            neco::yield(&i);
        }
    })();
    
    /*
     *neco::Result res = gen();
     *if (res != neco::Result::OK) {
     *    std::cout << "Error: " << (int)res << std::endl;
     *    return 1;
     *}
     */

    int i;
    while (gen.next(&i) != neco::Result::CLOSED) {
        std::cout << "Got: " << i << std::endl;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    // run main program in Neco coroutine context
    neco::Result ret = neco::run(argc, argv, main_);
    std::cout << "Main exit with code: " << (int)ret << std::endl;
}

