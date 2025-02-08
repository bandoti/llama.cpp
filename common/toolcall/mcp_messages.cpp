#include "mcpmessages.hpp"
#include <iostream>

using json = nlohmann::json;

const std::string mcp::JsonRpcVersion = "2.0";

void mcp::message::id(std::optional<nlohmann::json> id) {
    id_ = std::move(id);
}

const std::optional<nlohmann::json> & mcp::message::id() const {
    return id_;
}

json mcp::request::toJson() const override {
    json j;
    j["jsonrpc"] = JsonRpcVersion();
    j["method"] = method();
    j["id"] = id();
    if (params()) {
        j["params"] = params().value();
    }
    return j;
}

void mcp::request::method(std::string method) {
    method_ = std::move(method);
}

const std::string & mcp::request::method() const {
    return method_;
}

void mcp::request::params(std::optional<nlohmann::json> params) {
    params_ = std::move(params);
}

const std::optional<nlohmann::json> & mcp::request::params() const {
    return params_;
}

json mcp::response::error::toJson() const {
    json j;
    j["code"] = code;
    j["message"] = message;
    if (data) {
        j["data"] = data.value();
    }
    return j;
}

json mcp::response::toJson() const override {
    json j;
    j["jsonrpc"] = JsonRpcVersion();
    j["id"] = id();
    if (result()) {
        j["result"] = result().value();
    } else if (error()) {
        j["error"] = error()->toJson();
    }
    return j;
}

void mcp::response::result(std::optional<nlohmann::json> result) {
    result_ = std::move(result);
}

const std::optional<nlohmann::json> & mcp::response::result() const {
    return result_;
}

void mcp::response::error(std::optional<mcp::response::error> error) {
    error_ = std::move(error);
}

const std::optional<mcp::response::error> & mcp::response::error() const {
    return error_;
}

json mcp::notification::toJson() const override {
    json j;
    j["jsonrpc"] = JsonRpcVersion();
    j["method"] = method();
    if (params()) {
        j["params"] = params().value();
    }
    return j;
}

void mcp::notification::method(std::string method) {
    method_ = std::move(method);
}

const std::string & mcp::notification::method() const {
    return method_;
}

void mcp::notification::params(std::optional<nlohmann::json> params) {
    params_ = std::move(params);
}

const std::optional<nlohmann::json> & mcp::notification::params() const {
    return params_;
}

json mcp::client_capabilities::toJson() const {
    return json{
        {"samplingSupport", samplingSupport},
        {"notificationHandling", notificationHandling}
    };
}

json mcp::client_info::toJson() const {
    return json{
        {"name", name},
        {"version", version}
    };
}

std::optional<json> mcp::initialize_request::getParams() const override {
    return json{
        {"protocolVersion", protocolVersion},
        {"capabilities", capabilities.toJson()},
        {"clientInfo", clientInfo.toJson()}
    };
}

json mcp::server_capabilities::toJson() const {
    return json{
        {"resourceSubscriptions", resourceSubscriptions},
        {"toolSupport", toolSupport},
        {"promptTemplates", promptTemplates}
    };
}

mcp::server_capabilities mcp::server_capabilities::fromJson(const json& j) {
    mcp::server_capabilities caps;
    if (j.contains("resourceSubscriptions"))
        caps.resourceSubscriptions = j.at("resourceSubscriptions").get<bool>();

    if (j.contains("toolSupport"))
        caps.toolSupport = j.at("toolSupport").get<bool>();

    if (j.contains("promptTemplates"))
        caps.promptTemplates = j.at("promptTemplates").get<bool>();

    return caps;
}

json mcp::server_info::toJson() const {
    return json{
        {"name", name},
        {"version", version}
    };
}

mcp::server_info mcp::server_info::fromJson(const json& j) {
    mcp::server_info info;
    if (j.contains("name"))
        info.name = j.at("name").get<std::string>();

    if (j.contains("version"))
        info.version = j.at("version").get<std::string>();

    return info;
}

std::optional<json> mcp::initialize_response::getResult() const override {
    return json{
        {"protocolVersion", protocolVersion},
        {"capabilities", capabilities.toJson()},
        {"serverInfo", serverInfo.toJson()}
    };
}

mcp::initialize_response mcp::initialize_response::fromJson(const json& j) {
    return mcp::initialize_response(
        j.at("id").get<std::string>(),
        j.at("result").at("protocolVersion").get<std::string>(),
        mcp::server_capabilities::fromJson(j.at("result").at("capabilities")),
        mcp::server_info::fromJson(j.at("result").at("serverInfo"))
        );
}
