#pragma once

#include "mcp_transport.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>

struct CURL;

namespace toolcall
{
    class mcp_sse_transport : public mcp_transport {
    public:
        mcp_sse_transport(std::string server_uri);

        virtual void start() override;
        virtual void stop()  override;
        virtual bool send(const mcp::message_variant & request) override;

        size_t sse_read(const char * data, size_t len);

    private:
        void sse_run();
        void parse_field_value(std::string field, std::string value);
        void on_endpoint_event();
        void on_message_event();

        std::string server_uri_;
        bool running_;
        std::thread sse_thread_;
        CURL * endpoint_;

        struct sse_event {
            std::string type;
            std::string data;
            std::string id;
        } event_;

        std::string sse_buffer_;
        size_t sse_cursor_;
        std::string sse_last_id_;

        std::mutex initializing_mutex_;
        std::condition_variable initializing_;
    };
}
