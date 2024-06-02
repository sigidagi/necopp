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
    int serve(std::string_view network, std::string_view address);
    int serve(std::string_view network, std::string_view address, duration deadline);
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen, duration deadline);

    int dial(std::string_view network, std::string_view address);
    int dial(std::string_view network, std::string_view address, duration deadline);
   
    class io {
    public:
        io(int fd);
        ~io();
        std::vector<char> read(size_t size);
        std::vector<char> read(size_t size, duration deadline);
        ssize_t write(const std::vector<char>& buf);
        ssize_t write(std::string_view buf);
        ssize_t write(const std::vector<char>& buf, duration deadline);
    private:
        int m_fd;
    };

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
    class _receiver {
    public:
        void set(neco_chan* chan) {
            m_chan = chan;
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

        // Overload operator to receive data from channel
        const _receiver& operator>>(T& data) const {
            data = recv();
            return *this;
        }
    
    private:
        neco_chan*  m_chan = nullptr;
    };
    
    template<typename T>
    class _sender {
    public:
        void set(neco_chan* chan) {
            m_chan = chan;
        }

        Result send(const T& data) const {
            if (m_chan == nullptr) {
                return Result::INVAL;
            }
            return (Result)neco_chan_send(m_chan, &const_cast<T&>(data));
        }

        // Overload operator to send data to channel 
        const _sender& operator<<(const T& data) const {
            this->send(data);
            return *this;
        }
    private:
        neco_chan* m_chan = nullptr;
    };

    template<typename T>
    class channel {
    public:
        _sender<T> sender;
        _receiver<T> receiver;
 
        explicit channel(size_t capacity = 0) {
            neco_chan_make(&m_chan, sizeof(T), capacity);
            neco_chan_retain(m_chan);

            sender.set(m_chan);
            receiver.set(m_chan);
        }
   
        ~channel() {
            if (m_chan != nullptr) {
                neco_chan_release(m_chan);
            }
        }

    private:
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
        waitgroup();
        Result add(int delta);
        Result done();
        Result wait();
        Result wait(duration duration);
    private:
        neco_waitgroup m_waitgroup;
    };
} // namespace neco
