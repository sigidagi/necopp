#include "http.hpp"

#include <neco.h>
#include "error.hpp"
#include "http_parser.hpp"
#include <cstring>
#include <string>
#include <unistd.h>

namespace neco {

http::http(int sock) : sock_(sock) {
}

http::~http() {
    if (sock_ >= 0) {
        ::close(sock_);
    }
}

size_t http::send(const std::string& data) {

    if (sock_ < 0) {
        throw exception(http_error::SOCKET_CLOSED, "Socket is closed.");
    }
    size_t count = neco_write(sock_, data.c_str(), data.size());
    if (count < 0 || count != data.size()) {
        throw exception(http_error::SEND_FAILED, "Failed to send request.");
    }
    return count;
}

size_t http::write(const std::string& data) {
    return send(data);
}

response http::receive() {

    response response;
    http_parser parser;
    std::string currentHeaderField;
    std::string currentHeaderValue;

    parser.onHeaderField([&](const char* at, size_t length) {
        if (at == nullptr || length == 0) {
            throw exception(http_error::INVALID_HEADER_FIELD,
                    "Invalid header field (null pointer or zero length).");
        }
        currentHeaderField.assign(at, length); // Log the operation
        return 0;
    });

    parser.onHeaderValue([&](const char* at, size_t length) {
        if (at == nullptr || length == 0) {
            throw exception(http_error::INVALID_HEADER_VALUE,
                    "Invalid header value (null pointer or zero length).");
        }
        currentHeaderValue.assign(at, length); // Log the operation
        if (currentHeaderField == "Content-Length") {
            response.body.reserve(std::stoi(currentHeaderValue));  // Pre-allocate space for body
        }
        response.headers.emplace_back(currentHeaderField, currentHeaderValue); // Log the header pair
        currentHeaderField.clear();
        return 0;
    });

    parser.onBody([&](const char* at, size_t length) {
        if (at == nullptr || length == 0) {
            throw exception(http_error::INVALID_BODY, "Invalid body data (null pointer or zero length).");
        }

        try {
            response.body.append(at, length);
        } catch (const std::exception& e) {
            throw exception(http_error::INTERNAL_ERROR,
                    "Failed to append body data: " + std::string(e.what()));
        }
        return 0;
    });

    parser.onStatus([&](const char* at, size_t length) {
        if (at == nullptr || length == 0) {
            throw exception(http_error::INVALID_STATUS, "Invalid status line (null pointer or zero length).");
        }
        response.status.message.assign(at, length); // Extract the status line
        return 0;
    });

    parser.onMessageComplete([&]() {
        response.status.code = parser.statusCode();
        if (!currentHeaderField.empty()) {
            response.headers.emplace_back(currentHeaderField, currentHeaderValue);
        }

        return 0;
    });

    parse(&parser);
    return response;
}

void http::parse(http_parser* parser) {

    if (sock_ < 0) {
        throw exception(http_error::SOCKET_CLOSED, "Socket is closed.");
    }
    // Read response in chunks
    char buffer[1024];
    ssize_t bytes_received;

    while ((bytes_received = neco_read(sock_, buffer, sizeof(buffer))) > 0) {
        http_parser::error err = parser->execute(std::string_view(buffer, bytes_received));
        if (err != http_parser::error::OK) {
            throw exception(http_error::HTTP_PARSING_FAILED,
                    "HTTP parsing error: " + std::string(parser->errorReason()));
        }
    }

    if (bytes_received < 0) {
        throw exception(http_error::SOCKET_READ_FAILED,
                "Socket read error: " + std::string(strerror(errno)));
    }

    http_parser::error err = parser->finish();
    if (err != http_parser::error::OK) {
        throw exception(http_error::HTTP_PARSING_FAILED,
                "HTTP parsing error: " + std::string(parser->errorReason()));
    }
}

std::ostream& operator<<(std::ostream& os, const http_status& status) {
    os << "HTTP " << status.code << " " << status.message;
    return os;
}

}  // namespace neco
