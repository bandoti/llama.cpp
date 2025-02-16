#pragma once

#include "mcp_transport.hpp"
#include <thread>
#include <unordered_map>

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

        std::string server_uri_;
        bool running_;
        std::thread sse_thread_;
        CURL * sse_;
        CURL * endpoint_;

        struct sse_data {
            std::string data;
            size_t cursor;
            std::string event;
            std::string data;
            std::string id;
        } data_;

        std::string sse_data_;
        size_t sse_cursor_;
    };
}
