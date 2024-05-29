#pragma once

#include <vector>
#include <functional>
#include "neco.h"
#include <type_traits>
#include <chrono>
#include <string>
#include <iostream>

extern std::function<void(int, void**)>* globalFunction;

namespace neco {
     // Results from neco library
    enum class Result {
        OK = 0,
        ERROR = -1,
        INVAL = -2,
        PERM = -3,
        NOMEM = -4,
        ENDOF = -5,
        NOTFOUND = -6,
        NOSIGWATCH = -7,
        CLOSED = -8,
        EMPTY = -9,
        TIMEDOUT = -10,
        CANCELED = -11,
        BUSY = -12,
        NEGWAITGRP = -13,
        GAIERROR = -14,
        UNREADFAIL = -15,
        PARTIALWRITE = -16,
        NOTGENERATOR = -17,
        NOTSUSPENDED = -18
    };
   
    using coroutine = std::function<void(int, void**)>;
    using duration = std::chrono::duration<int64_t, std::ratio<1, 1000000000>>;
    using seconds = std::chrono::seconds;

    // C++ typedefs 
    /*
     *typedef neco_cond condition;
     *typedef neco_stream stream;
     *typedef neco_mutex mutex;
     */
    
    // Wrapper functions
    Result run(int argc, char* argv[], int (*user_main)(int, char**));

    template<typename T>
    void yield(T* data) { neco_gen_yield(data); }
    
    // Nano seconds duration 
    Result sleep(duration duration);
    Result sleep(duration duration, std::function<Result()> func);
    Result suspend();
    Result suspend(duration deadline); 
    Result resume(int64_t id);
    int64_t getid(); 
    int64_t lastid();
    int64_t starterid();
    void exit();
    
    // return file descriptor
    int serve(const char *network, const char *address);
    int serve(const char *network, const char *address, duration deadline);
    int dial(const char *network, const char *address);
    int dial(const char *network, const char *address, duration deadline);
    
    /*
     *Result select_impl(std::initializer_list<neco::channel*>&& chans);
     *template<typename... T>
     *Result select(T... chans) {
     *    return select_impl({std::forward<T>(chans)...});
     *}
     */

    // --------------------------------------------------------------------------------------------
    class go {
    public:
        explicit go(coroutine coroutine);
        virtual ~go() = default;

        template<typename... Args>
        Result operator()(Args... args) {
            constexpr int argc = sizeof...(Args);
            void (*callback)(int, void**) = convertToFunctionPointer(this->m_callback);
            return (Result)neco_start(callback, argc, static_cast<void*>(args)...);
        }
    protected:
        // Funciton to convert std::function to function pointer
        void (*convertToFunctionPointer(std::function<void(int, void**)>& func))(int, void**) {
            globalFunction = &func;
            return go::wrapper;
        }

        static void wrapper(int argc, void** argv);
        coroutine m_callback;
    private:
        //
    };
    
    template<typename T>
    class channel;

    template<typename T>
    struct channel_pair {
        channel<T> sender;
        channel<T> receiver;
    };

    template<typename T>
    class channel {
    public:
        static auto create(size_t capacity = 0) {
            neco::channel<T> chan1{capacity};
            neco::channel<T> chan2{chan1};
            return channel_pair<T>{
               chan1, 
               chan2,
            };
        }

        ~channel() {
            if (m_chan != nullptr) {
                neco_chan_release(m_chan);
            }
        }
       
        Result send(const T& data) const {
            if (m_chan == nullptr) {
                return Result::INVAL;
            }
            return (Result)neco_chan_send(m_chan, &const_cast<T&>(data));
        }

        T recv() const {
            // TODO provide return value/error handling
            T data{};
            if (m_chan == nullptr) {
                return data;
            }
            // return copy of data
            neco_chan_recv(m_chan, &data);
            return data;
        }

        // Overload operator to send data to channel 
        const channel& operator<<(const T& data) const {
            this->send(data);
            return *this;
        }
    
        // Overload operator to receive data from channel
        const channel& operator>>(T& data) const {
            data = recv();
            return *this;
        }

    private:
        explicit channel(size_t capacity = 0) {
            neco_chan_make(&m_chan, sizeof(T), capacity);
            neco_chan_retain(m_chan);
        }

        channel(const channel& other) {
            neco_chan_retain(other.m_chan);
            m_chan = other.m_chan;
        }
        channel& operator=(const channel& other) {
            if (this != &other) {
                neco_chan_retain(other.m_chan);
                m_chan = other.m_chan;
            }
            return *this;
        }

        channel() = delete;

        neco_chan * m_chan = nullptr;
    };
    
    template<typename T>
    class generator : public go {
    public:
        explicit generator(coroutine coro) : go(coro) {}

        ~generator() {
            if (m_gen != nullptr) {
                neco_gen_release(m_gen);
            }
        }

        template<typename... Args>
        generator& operator()(Args... args) {
            constexpr int argc = sizeof...(Args);
            void (*callback)(int, void**) = convertToFunctionPointer(this->m_callback);
            neco_gen_start(&m_gen, sizeof(T), callback, argc, static_cast<void*>(args)...);
            return *this;
        }

        neco_gen* get() {
            return m_gen;
        }
        
        Result next(T* data) {
            return (Result)neco_gen_next(m_gen, data);
        }

    private:
        neco_gen* m_gen = nullptr;
    };
   
    class waitgroup {
    public:
        waitgroup() {
            neco_waitgroup_init(&m_waitgroup);
        }
        
        Result add(int delta) {
            return (Result)neco_waitgroup_add(&m_waitgroup, delta);
        }

        Result done() {
            return (Result)neco_waitgroup_done(&m_waitgroup);
        }

        Result wait() {
            return (Result)neco_waitgroup_wait(&m_waitgroup);
        }

        Result wait(duration duration) {
            return (Result)neco_waitgroup_wait_dl(&m_waitgroup, duration.count());
        }

    private:
        neco_waitgroup m_waitgroup;
    };
} // namespace neco
