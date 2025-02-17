
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include "mcp_sse_transport.hpp"
#include <chrono>

toolcall::mcp_sse_transport::mcp_sse_transport(std::string server_uri)
    : server_uri_(std::move(server_uri)),
      running_(false),
      sse_thread_(),
      sse_(nullptr),
      endpoint_(nullptr),
      event_("", "", ""),
      sse_buffer_(""),
      sse_cursor_(0),
      sse_last_id_("")
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void toolcall::mcp_sse_transport::start() {
    if (running_) return;

    sse_thread_ = std::thread(&toolcall::mcp_sse_transport::sse_run, this);
    running_ = true;
}

void toolcall::mcp_sse_transport::stop() {
}

bool toolcall::mcp_sse_transport::send(const mcp::message_variant & /*request*/) {
    return false;
}

static size_t sse_callback(char * data, size_t size, size_t nmemb, void * clientp) {
    auto transport = static_cast<mcp_sse_transport*>(clientp);
    size_t len = size * nmemb;
    return transport->process_sse_data(data, len);
}

// Taken from specification:
// https://html.spec.whatwg.org/multipage/server-sent-events.html#event-stream-interpretation
//
void toolcall::mcp_sse_transport::parse_field_value(std::string field, std::string value) {
    if (field == "event") {
        // Set the event type buffer to field value.
        event_.type = std::move(value);

    } else if (field == "data") {
        // Append the field value to the data buffer,
        // then append a single U+000A LINE FEED (LF)
        // character to the data buffer.
        value += '\n';
        event_.data.insert(event_.data.end(), std::move(value));

    } else if (field == "id") {
        // If the field value does not contain U+0000 NULL,
        // then set the last event ID buffer to the field value.
        // Otherwise, ignore the field.
        if (! value.empty()) {
            event_.id = std::move(value);
        }

    } else if (field == "retry") {
        // If the field value consists of only ASCII digits,
        // then interpret the field value as an integer in base
        // ten, and set the event stream's reconnection time to
        // that integer. Otherwise, ignore the field.

        // TODO

    } else {
        // Ignore/log value
    }
}

void toolcall::mcp_sse_transport::on_endpoint_event() {
    endpoint_ = curl_easy_init();
    if (! endpoint_) {
        // Log error and abort
    }
    curl_easy_setopt(endpoint_, CURLOPT_URL, event_.data.c_str());
}

void toolcall::mcp_sse_transport::on_message_event() {
    mcp::message_variant message;
    if (mcp::create_message(event_.data, message)) {
        if (callback_) {
            callback_(message);
        }
    }
}

size_t toolcall::mcp_sse_transport::sse_read(const char * data, size_t len) {
    sse_buffer_.insert(sse_buffer_.end(), data, data + len);

    for (; sse_cursor_ < sse_buffer_.length(); ++sse_cursor_) {
        if (sse_buffer_[sse_cursor_] == '\r' || sse_buffer_[sse_cursor_] == '\n') {
            auto last = sse_buffer_.begin() + sse_cursor_;

            std::string line(sse_buffer_.begin(), last);
            if (line.empty()) { // Dispatch event
                if (event_.type == "endpoint") {
                    on_endpoint_event();

                } else if(event_.type == "message") {
                    on_message_event();

                } else {
                    // Ignore/log event
                }

                sse_last_id_ = event_.id;
                event_ = {"", "", ""};

            } else if(line[0] != ':') { // : denotes a comment
                // Set field/value
                auto found = line.find(':');
                if (found != line.end()) {
                    std::string field (line.begin(), found);
                    std::string value (found, line.end());

                    parse_field_value(std::move(field), std::move(value));
                }
            }

            if (last++ != sse_buffer_.end()) { // Consume line-end
                if (*last == '\n') {
                    last ++; // In the CRLF case consume one more
                }
                sse_buffer_ = std::string(last, sse_buffer_.end());

            } else {
                sse_buffer_.clear();
            }
            sse_cursor_ = 0; // Prepare to scan for next line-end
        }
    }
    return len;
}

void toolcall::mcp_sse_transport::sse_background() {
    char errbuf[CURL_ERROR_SIZE];
    size_t errlen;
    CURLMcode mcode;
    int num_handles;
    struct CURLMsg * m;
    int msgs_in_queue = 0;
    CURLM * async_handle = nullptr;
    struct curl_slist * headers = nullptr;
    CURL * sse = nullptr;

    sse = curl_easy_init();
    if (! sse) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        goto cleanup;
    }

    headers = curl_slist_append(headers, "Connection: keep-alive");

    curl_easy_setopt(sse, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(sse, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(sse, CURLOPT_URL, server_uri_.c_str());
    curl_easy_setopt(sse, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(sse, CURLOPT_WRITEFUNCTION, sse_callback);
    curl_easy_setopt(sse, CURLOPT_WRITEDATA, this);

    async_handle = curl_multi_init();
    if (! async_handle) {
        std::cerr << "Failed to initialize CURL async" << std::endl;
        goto cleanup;
    }
    curl_multi_add_handle(async_handle, sse);

    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(15));

        mcode = curl_multi_perform(async_handle, &num_handles);
        if (mcode != CURLM_OK) {
            std::cerr << curl_multi_strerror(mcode) << std::endl;
            break;
        }
        while ((m = curl_multi_info_read(async_handle, &msgs_in_queue)) != nullptr) {
            if (m->msg == CURLMSG_DONE) {
                if (m->data.result != CURLE_OK) {
                    errlen = strlen(errbuf);
                    if (errlen) {
                        std::cerr << errbuff << std::endl;
                    } else {
                        std::cerr << curl_easy_strerror(m->data.result) << std::endl;
                    }
                    running_ = false;
                    break;
                }
            }
        }

    } while (running_);

  cleanup:
    if (headers) {
        curl_slist_free_all(headers);
    }
    if (async_handle) {
        curl_multi_remove_handle(async_handle, sse);
        curl_multi_cleanup(async_handle);
    }
    if (sse) {
        curl_easy_cleanup(sse);
    }
}
