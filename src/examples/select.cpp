#include "necopp.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <fmt/format.h>

using namespace std::chrono_literals;

int main_(int, char **) {
    
    neco::channel<const char*> ch1;
    neco::channel<const char*> ch2;

    neco::go([](int, void** argv) {
        neco::channel<const char*>* ch = (neco::channel<const char*>*)argv[0];
        neco::sleep(500ms);
        ch->sender.send("Hello");
    })(&ch1);

    neco::go([](int, void** argv) {
        neco::channel<const char*>* ch = (neco::channel<const char*>*)argv[0];
        neco::sleep(1s);
        ch->sender.send("World");
    })(&ch2);
    
    /*
     *for (int i = 0; i < 2; i++) {
     *    switch (neco::select2(ch1, ch2)) {
     *        case 0:
     *            fmt::print("Received: '{}'\n", neco::channel_case(ch1));
     *            break;
     *        case 1:
     *            fmt::print("Received: '{}'\n", neco::channel_case(ch2));
     *            break;
     *        default:
     *            fmt::print("Error\n");
     *            break;
     *    }
     *}
     */
 
    neco::select(
        neco::incase{ch1, [](const auto &data) {
            fmt::print("Received: '{}'\n", data);
        }},
        neco::incase{ch2, [](const auto &data) {
            fmt::print("Received: '{}'\n", data);
        }}
    );
  
    return 0;
}

int main(int argc, char* argv[]) {
    // run main program in Neco coroutine context
    neco::run(argc, argv, main_);
}

