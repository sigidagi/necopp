#include <fmt/core.h>
#include <fmt/format.h>
#include "necopp.hpp"
#include <unistd.h>
#include <chrono>

using namespace std::chrono_literals;

void port_scan(int argc, void** argv) {
    (void)argc;
    int i = *(int*)argv[0];
    neco_waitgroup* wg = (neco_waitgroup*)argv[1];
    neco_waitgroup wg_ = *wg;
    // 
    std::string host = fmt::format("scanme.nmap.org:{}", i);

    int fd = neco::dial("tcp", host.c_str(), 1s);
    if (fd < 0) {
        fmt::print("Port {} closed\n", i);
        return;
    }
    fmt::print("Connected to {}\n", host);
    close(fd);
    neco_waitgroup_done(&wg_);
}

int main_(int argc, char** argv) {
    (void)argc;
    (void)argv;
    fmt::print("Hello, coroutine!\n");
    
    // synchronized counter - waitgroup
    neco_waitgroup wg;
    neco_waitgroup_init(&wg);
    
    for (int i = 79; i < 81; i++) {
        //        
        neco_waitgroup_add(&wg, 1);
        neco_start(port_scan, 2, &i, &wg);

/*
 *        neco::go([] (int argc, void** argv){
 *            (void)argc;
 *            int i = *(int*)argv[0];
 *            neco_waitgroup* wg = (neco_waitgroup*)argv[1];
 *            // 
 *            std::string host = fmt::format("scanme.nmap.org:{}", i);
 *
 *            int fd = neco::dial("tcp", host.c_str(), 1s);
 *            if (fd < 0) {
 *                fmt::print("Port {} closed\n", i);
 *                return;
 *            }
 *            fmt::print("Connected to {}\n", host);
 *            close(fd);
 *            neco_waitgroup_done(wg);
 *        })(&i, wg);
 */
    } // for loop
    
    neco_waitgroup_wait(&wg);
    return 0;
}

int neco_main(int argc, char** argv) {
    return main_(argc, argv);
}
/*
 *int main(int argc, char* argv[]) {
 *    neco::run(argc, argv, main_);
 *    return 0;
 *}
 */
