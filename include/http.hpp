#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace neco {

class http_parser;

struct http_status {
    int code = 0;               // HTTP status code (e.g., 200, 404)
    std::string message;        // HTTP status message (e.g., "OK", "Not Found")
};

std::ostream& operator<<(std::ostream& os, const http_status& status);

struct response {
    http_status status;
    std::vector<std::pair<std::string, std::string>> headers;
    std::string body;  // Raw response body
};

class http {
public:
    explicit http(int sock = -1);
    virtual ~http();
    size_t send(const std::string& data);
    size_t write(const std::string& data);

    response receive();
protected:
    int sock_;
private:
    //http_parser::type http_type_;
    void parse(http_parser* parser);
};

}  // namespace neco
