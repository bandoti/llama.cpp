#include "mcpmessages.hpp"
#include <iostream>

using json = nlohmann::json;

json mcp::request::toJson() const override {
    json j;
    j["jsonrpc"] = getJsonRpcVersion();
    j["method"] = getMethod();
    j["id"] = getId();
    if (getParams()) {
        j["params"] = getParams().value();
    }
    return j;
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
    j["jsonrpc"] = getJsonRpcVersion();
    j["id"] = getId();
    if (getResult()) {
        j["result"] = getResult().value();
    } else if (getError()) {
        j["error"] = getError()->toJson();
    }
    return j;
}

json mcp::notification::toJson() const override {
    json j;
    j["jsonrpc"] = getJsonRpcVersion();
    j["method"] = getMethod();
    if (getParams()) {
        j["params"] = getParams().value();
    }
    return j;
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
