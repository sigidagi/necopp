#pragma once 
#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

struct Timer {
    std::chrono::time_point<high_resolution_clock> start;
    Timer() : start(high_resolution_clock::now()) {}
    double elapsed() {
        auto end = high_resolution_clock::now();
        return std::chrono::duration_cast<milliseconds>(end - start).count();
    }
    ~Timer() = default;
};
