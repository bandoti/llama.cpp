
#include "handler.hpp"

toolcall::params::params(std::string tools, std::string choice) {
    this->tools(tools);
    this->choice(choice);
}

void toolcall::params::tools(std::string tools) {
    try {
        if (tools.empty() /*|| tools.beginswith("mcp+http")*/) {
            tools_ = std::move(tools);

        } else {
            tools_ = std::make_shared<json>(json::parse(tools));
            if (! tools_->is_array()) {
                throw std::invalid_argument("tools must be a valid JSON array");
            }
        }

    } catch (const json::exception & err) {
        throw std::invalid_argument(err.what());
    }
}

void toolcall::params::choice(std::string choice) {
    try {
        if (choice == "auto" || choice == "required" || choice == "none") {
            tool_choice_ = std::move(choice);

        } else {
            auto choice_ptr = std::make_shared<json>(json::parse(choice));
            tool_choice_ = choice_ptr;
            if (! choice_ptr->is_object()) {
                throw std::invalid_argument(
                    "tool choice must be a valid JSON object, \"auto\", \"required\", or \"none\"");
            }
        }

    } catch (const json::exception & err) {
        throw std::invalid_argument(err.what());
    }
}

toolcall::params::operator bool() const  {
    if (std::holds_alternative<std::string>(tools_)) {
        return ! std::get<std::string>(tools_).empty();

    } else {
        return std::get<json_ptr>(tools_) == nullptr;
    }
}

toolcall::handler::handler(std::unique_ptr<handler_impl> impl = nullptr)
    : impl_(std::move(impl))
{
}

void toolcall::handler::setImpl(std::unique_ptr<handler_imp> impl) {
    impl_ = std::move(impl);
}

json toolcall::handler::tool_list() {
    return impl_->tool_list();
}

json toolcall::handler::call(const json & tool_calls) {
    return impl_->call(tool_calls);
}

toolcall::loopback_impl::loopback_impl(on_tool_list tool_list_callback,
                                       on_call call_callback)

    : tool_list_callback_(tool_list_callback), call_callback_(call_callback)
{
}

json toolcall::loopback_impl::tool_list() {
    return tool_list_callback_();
}

json toolcall::loopback_impl::call(const json & tool_calls) {
    return call_callback_(tool_calls);
}


toolcall::mcp_impl::mcp_impl(std::string server_uri) {
}

json toolcall::mcp_impl::tool_list() {
    return json{}; // TODO
}

json toolcall::mcp_impl::call(const json & tool_calls) {
    return json{}; // TODO
}
