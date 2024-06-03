#include "hparser.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

std::function<int(llhttp_t *)>* globalFunction = nullptr;

hparser::hparser()
{
    llhttp_settings_init(&settings_);
}

int hparser::on_message(on_message_complete_t handle)
{
    llhttp_init(&parser_, HTTP_BOTH, &settings_);
    parser_.data = this;

    int (*callback)(llhttp_t*) = convertToFunctionPointer(handle);
    // on_message_complete
    settings_.on_message_complete = callback;

    return 0;
}

int hparser::wrapper(llhttp_t *parser) {
    if (globalFunction == nullptr) {
        return 0;
    }
    return (*globalFunction)(parser);
}

int hparser::execute(std::string_view data)
{
    return llhttp_execute(&parser_, data.data(), data.size());
}

