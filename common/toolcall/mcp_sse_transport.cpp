
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include "mcp_sse_transport.hpp"

toolcall::mcp_sse_transport::mcp_sse_transport(std::string server_uri)
    : server_uri_(std::move(server_uri)),
      running_(false),
      sse_thread_(),
      sse_(nullptr),
      endpoint_(nullptr),
      sse_data_(""),
      sse_cursor_(0)
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

size_t toolcall::mcp_sse_transport::sse_read(const char * data, size_t len) {
    sse_data_.insert(sse_data_.end(), data, data + len);

    for (; sse_cursor_ < sse_data_.length(); ++sse_cursor_) {
        if (sse_data_[sse_cursor_] == '\r' || sse_data_[sse_cursor_] == '\n') {
            auto last = sse_data_.begin() + sse_cursor_;

            std::string line(sse_data_.begin(), last);
            if (line.empty()) { // Dispatch command
                mcp::message_variant message;
                if (mcp::create_message(sse_data_, message)) {
                    if (callback_) {
                        callback_(message);
                    }
                }
                sse_data_.clear();

            } else if(line[0] != ':') { // Comments begin with :
                // Set field/value
                auto found = line.find(':'); // Field/Values delimited by :
                if (found != line.end()) {
                    std::string field (line.begin(), found);
                    std::string value (found, line.end());
                    if (field == "event") {
                        // Set the event type buffer to field value.

                    } else if (field == "data") {
                        // Append the field value to the data buffer,
                        // then append a single U+000A LINE FEED (LF)
                        // character to the data buffer.

                    } else if (field == "id") {
                        // If the field value does not contain U+0000 NULL,
                        // then set the last event ID buffer to the field value.
                        // Otherwise, ignore the field.

                    } else if (field == "retry") {
                        // If the field value consists of only ASCII digits,
                        // then interpret the field value as an integer in base
                        // ten, and set the event stream's reconnection time to
                        // that integer. Otherwise, ignore the field.

                    } else {
                        // Ignore/log value
                    }
                }
            }

            bool has_more = sse_cursor_ + 1 < sse_data_.length();
            if (has_more) {
                if (*(last + 1) == '\n') { // In the CRLF case
                    last ++;
                }
                sse_data_ = std::string(last, sse_data_.end());

            } else {
                sse_data_.clear();
            }
            sse_cursor_ = 0;
        }
    }
    return len;
}

void toolcall::mcp_sse_transport::sse_background() {
    // This is only place the sse_ member should be accessed.
    // Set options using curl_easy_setopt
    // Set up write function which handles the events
    sse_ = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return;
    }

    curl_easy_setopt(sse_, CURLOPT_URL, server_uri_.c_str());
    curl_easy_setopt(sse_, CURLOPT_WRITEFUNCTION, sse_callback);
    curl_easy_setopt(sse_, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(sse_, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(sse_, CURLOPT_HTTPHEADER, headers_.get());

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    }

    curl_easy_cleanup(curl);
}
