#include "hparser.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

std::function<int(llhttp_t *)>* globalParserFunction = nullptr;

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
    if (globalParserFunction == nullptr) {
        return 0;
    }
    return (*globalParserFunction)(parser);
}

int hparser::execute(std::string_view data)
{
    return llhttp_execute(&parser_, data.data(), data.size());
}

uint8_t hparser::status_code() const
{
    return parser_.status_code;
}

uint8_t hparser::method() const
{
    return parser_.method;
}

uint64_t hparser::content_length() const
{
    return parser_.content_length;
}

