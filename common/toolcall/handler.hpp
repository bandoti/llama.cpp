#pragma once

#include <string>
#include <variant>
#include <memory>

#include "../json.hpp"

namespace toolcall
{
    using json = nlohmann::ordered_json;

    class params {
    public:
        using json_ptr = std::shared_ptr<json>;
        using tools_t = std::variant<std::string, json_ptr>;
        using tool_choice_t = std::variant<std::string, json_ptr>;

        params(std::string tools = "", std::string choice = "auto");

        params(const params & other) = default;
        params(params && other) noexcept = default;
        params & operator=(const params & other) = default;
        params & operator=(params && other) noexcept = default;

        operator bool() const;

        void tools(std::string tools);
        const tools_t tools() const { return tools_; }

        void choice(std::string choice);
        const tool_choice_t & choice() const { return tool_choice_; }

    private:
        tools_t tools_;
        tool_choice_t tool_choice_;
    };

    class handler_impl;
    class handler {
    public:
        handler(std::unique_ptr<handler_impl> impl = nullptr);
        void setImpl(std::unique_ptr<handler_imp> impl);
        json tool_list();
        json call(const json & tool_calls);

    private:
        std::unique_ptr<handler_impl> impl_;
    };

    class handler_impl {
    public:
        virtual ~handler_impl() = default;

        virtual json tool_list() = 0;
        virtual json call(const json & tool_calls) = 0;
    };

    class loopback_impl : public handler_impl {
    public:
        using on_tool_list = std::function<json(void)>;
        using on_call = std::function<json(json)>;

        loopback_impl(on_tool_list tool_list_callback, on_call call_callback);

        virtual json tool_list() override;
        virtual json call(const json & tool_calls) override;

    private:
        on_tool_list tool_list_callback_;
        on_call call_callback_;
    };

    class mcp_impl : public handler_impl {
    public:
        mcp_impl(std::string server_uri);

        virtual json tool_list() override;
        virtual json call(const json & tool_calls) override;
    };
}
