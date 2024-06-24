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

    neco::go([&](int, void**) {
        neco::sleep(500ms);
        ch2.sender.send("Hello");
    })();

    neco::go([&](int, void**) {
        neco::sleep(1s);
        ch1.sender.send("World");
    })();
    
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
    
    std::vector<std::string> words;
    neco::select(
        neco::incase{ch1, [&words](const auto &data) {
            words.push_back(data);
        }},
        neco::incase{ch2, [&words](const auto &data) {
            words.push_back(data);
        }}
    );
    
    fmt::print("Words: '");
    for (const auto &word : words) {
        fmt::print("{} ", word);
    }
    fmt::print("'\n");
    fflush(stdout);
    
    return 0;
}

int main(int argc, char* argv[]) {
    // run main program in Neco coroutine context
    neco::run(argc, argv, main_);
}

