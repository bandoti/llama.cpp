#include <iostream>
#include <string>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ------------------------
// Base Interfaces
// ------------------------

class IMessage {
public:
    virtual ~IMessage() = default;

    virtual std::string getJsonRpcVersion() const { return "2.0"; }
    virtual json toJson() const = 0;
};

class IRequest : public IMessage {
public:
    virtual ~IRequest() = default;

    virtual std::string getMethod() const = 0;
    virtual json getId() const = 0;
    virtual std::optional<json> getParams() const = 0;

    json toJson() const override {
        json j;
        j["jsonrpc"] = getJsonRpcVersion();
        j["method"] = getMethod();
        j["id"] = getId();
        if (getParams()) {
            j["params"] = getParams().value();
        }
        return j;
    }
};

class IResponse : public IMessage {
public:
    struct Error {
        int code;
        std::string message;
        std::optional<json> data;

        json toJson() const {
            json j;
            j["code"] = code;
            j["message"] = message;
            if (data) {
                j["data"] = data.value();
            }
            return j;
        }
    };

    virtual ~IResponse() = default;

    virtual json getId() const = 0;
    virtual std::optional<json> getResult() const = 0;
    virtual std::optional<Error> getError() const = 0;

    json toJson() const override {
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
};

class INotification : public IMessage {
public:
    virtual ~INotification() = default;

    virtual std::string getMethod() const = 0;
    virtual std::optional<json> getParams() const = 0;

    json toJson() const override {
        json j;
        j["jsonrpc"] = getJsonRpcVersion();
        j["method"] = getMethod();
        if (getParams()) {
            j["params"] = getParams().value();
        }
        return j;
    }
};

// ------------------------
// Client Capability Structures
// ------------------------

struct ClientCapabilities {
    bool samplingSupport = false;
    bool notificationHandling = false;

    json toJson() const {
        return json{
            {"samplingSupport", samplingSupport},
            {"notificationHandling", notificationHandling}
        };
    }
};

struct ClientInfo {
    std::string name;
    std::string version;

    json toJson() const {
        return json{
            {"name", name},
            {"version", version}
        };
    }
};

// ------------------------
// Initialize Request (Client → Server)
// ------------------------

class InitializeRequest : public IRequest {
private:
    std::string id;
    std::string protocolVersion;
    ClientCapabilities capabilities;
    ClientInfo clientInfo;

public:
    InitializeRequest(std::string requestId, std::string version, ClientCapabilities caps, ClientInfo info)
        : id(std::move(requestId)), protocolVersion(std::move(version)), capabilities(std::move(caps)), clientInfo(std::move(info)) {}

    std::string getMethod() const override { return "initialize"; }
    json getId() const override { return id; }
    std::optional<json> getParams() const override {
        return json{
            {"protocolVersion", protocolVersion},
            {"capabilities", capabilities.toJson()},
            {"clientInfo", clientInfo.toJson()}
        };
    }
};

// ------------------------
// Initialize Response (Server → Client)
// ------------------------

struct ServerCapabilities {
    bool resourceSubscriptions = false;
    bool toolSupport = false;
    bool promptTemplates = false;

    json toJson() const {
        return json{
            {"resourceSubscriptions", resourceSubscriptions},
            {"toolSupport", toolSupport},
            {"promptTemplates", promptTemplates}
        };
    }

    static ServerCapabilities fromJson(const json& j) {
        ServerCapabilities caps;
        if (j.contains("resourceSubscriptions")) caps.resourceSubscriptions = j.at("resourceSubscriptions").get<bool>();
        if (j.contains("toolSupport")) caps.toolSupport = j.at("toolSupport").get<bool>();
        if (j.contains("promptTemplates")) caps.promptTemplates = j.at("promptTemplates").get<bool>();
        return caps;
    }
};

struct ServerInfo {
    std::string name;
    std::string version;

    json toJson() const {
        return json{
            {"name", name},
            {"version", version}
        };
    }

    static ServerInfo fromJson(const json& j) {
        ServerInfo info;
        if (j.contains("name")) info.name = j.at("name").get<std::string>();
        if (j.contains("version")) info.version = j.at("version").get<std::string>();
        return info;
    }
};

class InitializeResponse : public IResponse {
private:
    std::string id;
    std::string protocolVersion;
    ServerCapabilities capabilities;
    ServerInfo serverInfo;

public:
    InitializeResponse(std::string responseId, std::string version, ServerCapabilities caps, ServerInfo info)
        : id(std::move(responseId)), protocolVersion(std::move(version)), capabilities(std::move(caps)), serverInfo(std::move(info)) {}

    json getId() const override { return id; }
    std::optional<json> getResult() const override {
        return json{
            {"protocolVersion", protocolVersion},
            {"capabilities", capabilities.toJson()},
            {"serverInfo", serverInfo.toJson()}
        };
    }

    static InitializeResponse fromJson(const json& j) {
        return InitializeResponse(
            j.at("id").get<std::string>(),
            j.at("result").at("protocolVersion").get<std::string>(),
            ServerCapabilities::fromJson(j.at("result").at("capabilities")),
            ServerInfo::fromJson(j.at("result").at("serverInfo"))
        );
    }
};

// ------------------------
// Initialized Notification (Client → Server)
// ------------------------

class InitializedNotification : public INotification {
public:
    std::string getMethod() const override { return "initialized"; }
    std::optional<json> getParams() const override { return std::nullopt; }
};

// ------------------------
// Usage Example
// ------------------------

int main() {
    // Create client capabilities
    ClientCapabilities clientCaps;
    clientCaps.samplingSupport = true;
    clientCaps.notificationHandling = true;

    // Create client info
    ClientInfo clientInfo;
    clientInfo.name = "MyMCPClient";
    clientInfo.version = "1.0.0";

    // Create initialize request
    InitializeRequest initReq("1", "1.0", clientCaps, clientInfo);

    // Serialize request to JSON
    json initReqJson = initReq.toJson();
    std::cout << "Initialize Request JSON:\n" << initReqJson.dump(2) << "\n\n";

    // Simulated server response JSON
    json serverRespJson = R"({
        "jsonrpc": "2.0",
        "id": "1",
        "result": {
            "protocolVersion": "1.0",
            "capabilities": {
                "resourceSubscriptions": true,
                "toolSupport": false,
                "promptTemplates": true
            },
            "serverInfo": {
                "name": "MCPServer",
                "version": "1.2.3"
            }
        }
    })"_json;

    // Parse response
    InitializeResponse initResp = InitializeResponse::fromJson(serverRespJson);
    std::cout << "Server Info: " << initResp.toJson().dump(2) << "\n";

    return 0;
}
