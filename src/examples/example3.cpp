#include "necopp.hpp"
#include <memory>
#include <iostream>

using namespace std::chrono_literals;

int main_(int, char **) {

    int arg0 = 0;
    auto arg1 = std::make_shared<int>(1);
    std::string arg2 = "hello world";

    neco::go([&arg0, &arg1, &arg2] (int, void** ) {
        // 
        std::cout << "arg0: " << arg0 << ", arg1: " << *arg1 << "', arg2: " << arg2 << "\n";
        neco::sleep(500ms);
        std::cout << "second done\n";
    })();
    
    // or 
    neco::go([] (int, void** argv) {
    
        int* arg0 = static_cast<int*>(argv[0]);
        int* arg1 = static_cast<int*>(argv[1]);
        const char* arg2 = static_cast<const char*>(argv[2]);

        std::cout << "arg0: " << *arg0 << ", arg1: " << *arg1 << "', arg2: " << arg2 << "\n";
        neco::sleep(500ms);
        std::cout << "third done\n";
    })(&arg0, arg1.get(), arg2.data());
   
    neco::sleep(1s);
    std::cout << "first done\n";
    return 0;
}

int main(int argc, char* argv[]) {
    return (int)neco::run(argc, argv, main_);
}
