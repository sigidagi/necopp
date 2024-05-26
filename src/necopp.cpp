#include "necopp.hpp"
#include <iostream>

std::function<void(int, void**)>* globalFunction = nullptr;

namespace neco 
{
    // Nano seconds duration 
    Result sleep(duration duration) { return (Result)neco_sleep(duration.count()); }
    Result sleep(duration duration, std::function<Result()> func) {
        neco::sleep(duration);
        return func();
    }

    Result suspend() { return (Result)neco_suspend(); }
    Result suspend(duration deadline) {return (Result)neco_suspend_dl(deadline.count()); }
    Result resume(int64_t id) { return (Result)neco_resume(id); }
    int64_t getid() { return neco_getid(); }
    int64_t lastid() { return neco_lastid(); }
    int64_t starterid() { return neco_starterid();}
    void exit() { neco_exit();}

    int serve(const char *network, const char *address) {
        return neco_serve(network, address);
    }

    int serve(const char *network, const char *address, duration deadline) {
        return neco_serve_dl(network, address, deadline.count());
    }

    int dial(const char *network, const char *address) {
        return neco_dial(network, address);
    }
    
    int dial(const char *network, const char *address, duration deadline) {
        seconds sec = std::chrono::duration_cast<seconds>(deadline);
        int64_t dl = neco_now() + sec.count()*1000000000;
        return neco_dial_dl(network, address, dl);
    }

/*
 *    Result select_impl(std::initializer_list<neco::channel*>&& chans) {
 *        return (Result)neco_chan_select(chans.size(), chans.begin());
 *    }
 *
 */
    // --------------------------------------------------------------------------------------------
    go::go(coroutine coro) : m_callback(coro) {}

    void go::wrapper(int argc, void** argv) {
        if (globalFunction == nullptr) {
            return;
        }
        (*globalFunction)(argc, argv);
    }
    
    Result run(int argc, char* argv[], int (*user_main)(int, char**)) {
        neco_env_setpaniconerror(true);
        neco_env_setcanceltype( NECO_CANCEL_ASYNC ); 
        
        auto coro = neco::go([user_main](int argc, void** argv) {
            (void)argc;
            __neco_exit_prog(user_main(*(int*)argv[0], *(char***)argv[1]));
        });
        
        return coro(&argc, &argv);
    }

} // namespace neco

    /*
     *void adapter(int argc, void** argv) {
     *    auto func = *reinterpret_cast<std::function<void(int, void**)>*>(argv[0]);
     *    void** args = reinterpret_cast<void**>(argv[1]);
     *    func(argc -1, args);
     *}
     */

