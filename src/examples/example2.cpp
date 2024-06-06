#include "necopp.hpp"
#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

int main_(int, char **) {
   // 
    neco::go([](int, void **){
        while (true) {
            neco::sleep(1s);
            std::cout << "tick\n";
        }
    })();

    neco::go([](int, void **){
        while (true) {
            neco::sleep(1s);
            std::cout << "tock...\n";
        }
    })();

    // Keep the program alive for an hour.
    neco::sleep(1h);
    return 0;
}

int main(int argc, char* argv[]) {
    return (int)neco::run(argc, argv, main_);
}
