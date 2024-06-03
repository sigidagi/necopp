#ifndef LLHTTPP_HPP
#define LLHTTPP_HPP

#include "llhttp.h"

class HttpParser {
    public:
        HttpParser();

    // on_message_complete 
    int on_message(llhttp_t* parser);

    private:
        llhttp_t parser_;
        llhttp_settings_t settings_;
};

#endif /* LLHTTPP_HPP */
