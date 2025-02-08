#include "mcpmessages.hpp"
#include <iostream>

using json = nlohmann::json;

const std::string mcp::JsonRpcVersion = "2.0";
const std::string mcp::McpVersion     = "2024-11-05";
const std::string mcp::ClientVersion  = "1.0.0";

mcp::message::message(std::optional<nlohmann::json> id) : id_(std::move(id))
{
}

void mcp::message::id(std::optional<nlohmann::json> id) {
    id_ = std::move(id);
}

const std::optional<nlohmann::json> & mcp::message::id() const {
    return id_;
}

mcp::request::request(std::optional<nlohmann::json> id,
                      std::string method,
                      std::optional<nlohmann::json> params)

    : message(id), method_(std::move(method)), params_(std::move(params))
{
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

mcp::response::response(std::optional<nlohmann::json> id,
                        std::optional<nlohmann::json> result = std::nullopt,
                        std::optional<error> error = std::nullopt)

    : message(id), result_(result), error_(error)
{
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

mcp::notification::notification(std::optional<nlohmann::json> id,
             std::string method,
             std::optional<nlohmann::json> params = std::nullopt)

    : message(id), method_(method), params_(params)
{
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

json capability::toJson() const {
    json cap {{name, {}}};
    if (subscribe) {
        cap[name]["subscribe"] = true;
    }
    if (listChanged) {
        cap[name]["listChanged"] = true;
    }
    return cap;
}

mcp::initialize_request::initialize_request(nlohmann::json id, capabilities caps)
    : request(id, "initialize"), caps_(std::move(caps))
{
    json params;
    params["protocolVersion"] = mcp::McpVersion;
    params["clientInfo"]["name"] = "llama.cpp";
    params["clientInfo"]["version"] = mcp::ClientVersion;
    params["capabilities"] = {};

    for (auto cap = caps.cbegin(); cap != caps.cend(); ++cap) {
        json cap_json;

        if (cap->subscribe) {
            cap_json["subscribe"] = true;
        }
        if (cap->listChanged) {
            cap_json["listChanged"] = true;
        }

        params["capabilities"][cap->name] = cap_json;
    }

    params(params);
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
