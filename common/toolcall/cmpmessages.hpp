#include <string>
#include <optional>
#include <nlohmann/json.hpp>

class mcp_message {
public:
    virtual ~mcp_message() = default;

    virtual std::string getJsonRpcVersion() const {
        return "2.0";
    }

    virtual std::optional<nlohmann::json> getId() const = 0;

    virtual nlohmann::json toJson() const = 0;
};

class mcp_request : public mcp_message {
public:
    virtual ~mcp_request() = default;

    virtual std::string getMethod() const = 0;
    virtual std::optional<nlohmann::json> getParams() const = 0;

    nlohmann::json toJson() const override;
};

class mcp_response : public mcp_message {
public:
    struct error {
        int code;
        std::string message;
        std::optional<nlohmann::json> data;

        nlohmann::json toJson() const;
    };

    virtual ~mcp_response() = default;

    virtual std::optional<nlohmann::json> getResult() const = 0;
    virtual std::optional<error> getError() const = 0;

    nlohmann::json toJson() const override;
};

class mcp_notification : public mcp_message {
public:
    virtual ~mcp_notification() = default;

    virtual std::string getMethod() const = 0;
    virtual std::optional<nlohmann::json> getParams() const = 0;

    nlohmann::json toJson() const override;
};

struct mcp_client_capabilities {
    bool samplingSupport = false;
    bool notificationHandling = false;

    nlohmann::json toJson() const;
};

struct mcp_client_info {
    std::string name;
    std::string version;

    nlohmann::json toJson() const;
};

class mcp_initialize_request : public mcp_request {
private:
    std::string id;
    std::string protocolVersion;
    mcp_client_capabilities capabilities;
    mcp_client_info clientInfo;

public:
    mcp_initialize_request(std::string requestId,
                           std::string version,
                           mcp_client_capabilities caps,
                           mcp_client_info info)

        : id(std::move(requestId)),
          protocolVersion(std::move(version)),
          capabilities(std::move(caps)),
          clientInfo(std::move(info))
        {}

    std::string getMethod() const override { return "initialize"; }
    nlohmann::json getId() const override { return id; }
    std::optional<json> getParams() const override;
};

struct mcp_server_capabilities {
    bool resourceSubscriptions = false;
    bool toolSupport = false;
    bool promptTemplates = false;

    nlohmann::json toJson() const;

    static mcp_server_capabilities fromJson(const json& j);
};

struct mcp_server_info {
    std::string name;
    std::string version;

    nlohmann::json toJson() const;

    static mcp_server_info fromJson(const json& j);
};

class mcp_initialize_response : public mcp_response {
private:
    std::string id;
    std::string protocolVersion;
    mcp_server_capabilities capabilities;
    mcp_server_info serverInfo;

public:
    mcp_initialize_response(std::string responseId,
                            std::string version,
                            mcp_server_capabilities caps,
                            mcp_server_info info)

        : id(std::move(responseId)),
          protocolVersion(std::move(version)),
          capabilities(std::move(caps)),
          serverInfo(std::move(info))
        {}

    nlohmann::json getId() const override { return id; }
    std::optional<json> getResult() const override;

    static mcp_initialize_response fromJson(const json& j);
};

class mcp_initialized_notification : public mcp_notification {
public:
    std::string getMethod() const override { return "initialized"; }
    std::optional<json> getParams() const override { return std::nullopt; }
};

// ------------------------
// Usage Example
// ------------------------
/*
int main() {
    // Create client capabilities
    mcp_client_capabilities clientCaps;
    clientCaps.samplingSupport = true;
    clientCaps.notificationHandling = true;

    // Create client info
    mcp_client_info clientInfo;
    clientInfo.name = "MyMCPClient";
    clientInfo.version = "1.0.0";

    // Create initialize request
    mcp_initialize_request initReq("1", "1.0", clientCaps, clientInfo);

    // Serialize request to JSON
    nlohmann::json initReqJson = initReq.toJson();
    std::cout << "Initialize Request JSON:\n" << initReqJson.dump(2) << "\n\n";

    // Simulated server response JSON
    nlohmann::json serverRespJson = R"({
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
    mcp_initialize_response initResp = mcp_initialize_response::fromJson(serverRespJson);
    std::cout << "Server Info: " << initResp.toJson().dump(2) << "\n";

    return 0;
}
*/
