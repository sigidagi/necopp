#ifndef HPARSER_HPP
#define HPARSER_HPP

#include "llhttp.h"
#include <functional>
#include <string_view>
#include <string>
#include <map>
#include <fmt/format.h>


namespace httpp {
    
    enum class method {
        GET,
        POST,
        PUT,
        DELETE,
        HEAD,
        CONNECT,
        OPTIONS,
        TRACE,
        PATCH,
        UNKNOWN
    };
    
    enum class type {
        REQUEST,
        RESPONSE
    };

    typedef struct {
        uint8_t status_code;
        uint64_t content_length;
        std::string body;
        std::map<std::string, std::string> headers;
    } response_t;


    class parser {
    public:
        parser() {
            llhttp_settings_init(&settings_);

            settings_.on_message_complete = [](llhttp_t *) -> int {
                fmt::print("Message complete\n");
                return 0;
            };

            settings_.on_header_field = [](llhttp_t* parser, const char* at, size_t length) {
                
                //fmt::print("Header field: {}\n", std::string(at, length));
                response_t* r = static_cast<response_t*>(parser->data);
                r->headers.emplace(std::string(at, length), "");
                return 0;
            };
    
            settings_.on_header_value = [](llhttp_t* parser, const char* at, size_t length) {
                
                response_t* r = static_cast<response_t*>(parser->data);
                auto it = std::find_if(r->headers.begin(), r->headers.end(), [](const auto& pair) {
                    return pair.second == "";
                });
                
                if (it != r->headers.end()) {
                    it->second = std::string(at, length);
                }
                else {
                    // TODO
                    fmt::print("Error: no header field found\n");
                    return -1;
                }

                return 0;
            };

            settings_.on_body = [](llhttp_t* parser, const char* at, size_t length) {
                
                response_t *r = static_cast<response_t*>(parser->data);
                r->body.append(at, length);
                return 0;
            };


            llhttp_init(&parser_, HTTP_BOTH, &settings_);
            // only after init
            parser_.data = &res_;
        }

        int execute(std::string_view data) {
            return llhttp_execute(&parser_, data.data(), data.size());
        }

        uint8_t status_code() const {
            return parser_.status_code;
        }

        uint64_t content_length() const {
            return parser_.content_length;
        }

        response_t response() const {
            return res_;
        }

    private:
        llhttp_t parser_;
        llhttp_settings_t settings_;

        response_t res_;
    };



} // namespace httpp


#endif /* HPARSER_HPP */
