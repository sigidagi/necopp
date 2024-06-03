#ifndef HPARSER_HPP
#define HPARSER_HPP

#include "llhttp.h"
#include <functional>
#include <string_view>

extern std::function<int(llhttp_t *parser)>* globalFunction;
typedef std::function<int(llhttp_t *)> on_message_complete_t;

class hparser {
    public:
        hparser();

    // on_message_complete 
    int on_message(on_message_complete_t handle);
    int execute(std::string_view data);

    protected:
        // Funciton to convert std::function to function pointer
        int (*convertToFunctionPointer(std::function<int(llhttp_t*)>& func))(llhttp_t*) {
            globalFunction = &func;
            return hparser::wrapper;
        }

        static int wrapper(llhttp_t *parser);

    private:
        llhttp_t parser_;
        llhttp_settings_t settings_;
};

#endif /* HPARSER_HPP */
