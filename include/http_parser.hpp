#pragma once

#include <llhttp.h>
#include <functional>
#include <string>
#include <string_view>

namespace neco {

class http_parser {
public:
    using callback = std::function<int()>;
    using data_callback = std::function<int(const char*, size_t)>;

    enum class error {
        OK = HPE_OK,
        INTERNAL = HPE_INTERNAL,
        STRICT = HPE_STRICT,
        PAUSED = HPE_PAUSED
    };

    enum class type {
        REQUEST = HTTP_REQUEST,
        RESPONSE = HTTP_RESPONSE,
        BOTH = HTTP_BOTH
    };

    http_parser() {
        llhttp_settings_init(&settings_);
        
        // TODO - Add support for HTTP_REQUEST and HTTP_RESPONSE
        llhttp_init(&parser_, static_cast<llhttp_type_t>(type::BOTH), &settings_);
        parser_.data = this;

        settings_.on_message_begin = [](llhttp_t* parser) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onMessageBegin_ ? self->onMessageBegin_() : 0;
        };

        settings_.on_message_complete = [](llhttp_t* parser) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onMessageComplete_ ? self->onMessageComplete_() : 0;
        };

        settings_.on_header_field = [](llhttp_t* parser, const char* at, size_t length) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onHeaderField_ ? self->onHeaderField_(at, length) : 0;
        };

        settings_.on_header_value = [](llhttp_t* parser, const char* at, size_t length) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onHeaderValue_ ? self->onHeaderValue_(at, length) : 0;
        };

        settings_.on_url = [](llhttp_t* parser, const char* at, size_t length) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onUrl_ ? self->onUrl_(at, length) : 0;
        };

        settings_.on_body = [](llhttp_t* parser, const char* at, size_t length) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onBody_ ? self->onBody_(at, length) : 0;
        };

        settings_.on_status = [](llhttp_t* parser, const char* at, size_t length) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onStatus_ ? self->onStatus_(at, length) : 0;
        };
    }
    ~http_parser() = default;

    void onMessageBegin(callback callback) {
        onMessageBegin_ = std::move(callback);
    }

    void onMessageComplete(callback callback) {
        onMessageComplete_ = std::move(callback);
    }

    void onHeaderField(data_callback callback) {
        onHeaderField_ = std::move(callback);
    }

    void onHeaderValue(data_callback callback) {
        onHeaderValue_ = std::move(callback);
    }

    void onUrl(data_callback callback) {
        onUrl_ = std::move(callback);
    }

    void onBody(data_callback callback) {
        onBody_ = std::move(callback);
    }

    void onStatus(data_callback callback) {
        onStatus_ = std::move(callback);
    }

    http_parser::error execute(std::string_view data) {
        llhttp_errno_t err = llhttp_execute(&parser_, data.data(), data.size());
        return static_cast<error>(err);
    }

    http_parser::error finish() {
        llhttp_errno_t err = llhttp_finish(&parser_);
        return static_cast<error>(err);
    }

    const char* errorReason() const {
        return llhttp_errno_name(static_cast<llhttp_errno_t>(parser_.error));
    }

    int statusCode() const {
        return parser_.status_code;
    }

private:
    llhttp_t parser_;
    llhttp_settings_t settings_;

    callback onMessageBegin_;
    callback onMessageComplete_;

    data_callback onHeaderField_;
    data_callback onHeaderValue_;
    data_callback onUrl_;
    data_callback onBody_;
    data_callback onStatus_;
};

} // namespace neco
