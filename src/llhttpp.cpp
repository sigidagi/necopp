#include "llhttpp.hpp"

#include <fmt/core.h>
#include <fmt/format.h>


HttpParser::HttpParser()
{
    llhttp_settings_init(&settings_);
}

int HttpParser::on_message(llhttp_t *)
{
    fmt::print("message completed!\n");
    return 0;
}
