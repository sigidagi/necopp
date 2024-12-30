#pragma once

#include <exception>
#include <string>

namespace neco {

enum class http_error {
    OK,
    SOCKET_INIT_FAILED,
    CONNECTION_FAILED,
    SOCKET_READ_FAILED,
    HTTP_PARSING_FAILED,
    SEND_FAILED,
    INVALID_URL,
    INVALID_HEADER_FIELD,
    INVALID_HEADER_VALUE,
    INVALID_BODY,
    INVALID_STATUS,
    INTERNAL_ERROR
};

class exception : public std::exception {
public:
    exception(http_error error, std::string message)
        : error_(error), message_(std::move(message)) {}

    // Override the what() method to provide the error message
    const char* what() const noexcept override {
        return message_.c_str();
    }

    http_error error() const noexcept { return error_; }
    const std::string& message() const noexcept { return message_; }

private:
    http_error error_;
    std::string message_;
};

} // namespace neco
