#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace mcp
{
    class message {
    public:
        virtual ~message() = default;
        virtual std::string getJsonRpcVersion() const {
            return "2.0";
        }
        virtual std::optional<nlohmann::json> getId() const = 0;
        virtual nlohmann::json toJson() const = 0;
    };

    class request : public message {
    public:
        virtual ~request() = default;
        virtual std::string getMethod() const = 0;
        virtual std::optional<nlohmann::json> getParams() const = 0;
        nlohmann::json toJson() const override;
    };

    class response : public message {
    public:
        struct error {
            int code;
            std::string message;
            std::optional<nlohmann::json> data;
            nlohmann::json toJson() const;
        };
        virtual ~response() = default;
        virtual std::optional<nlohmann::json> getResult() const = 0;
        virtual std::optional<error> getError() const = 0;
        nlohmann::json toJson() const override;
    };

    class notification : public message {
    public:
        virtual ~notification() = default;
        virtual std::string getMethod() const = 0;
        virtual std::optional<nlohmann::json> getParams() const = 0;
        nlohmann::json toJson() const override;
    };

    struct client_capabilities {
        bool samplingSupport = false;
        bool notificationHandling = false;
        nlohmann::json toJson() const;
    };

    struct client_info {
        std::string name;
        std::string version;
        nlohmann::json toJson() const;
    };

    class initialize_request : public request {
    private:
        std::string id;
        std::string protocolVersion;
        client_capabilities capabilities;
        client_info clientInfo;

    public:
        initialize_request(std::string requestId,
                               std::string version,
                               client_capabilities caps,
                               client_info info)

            : id(std::move(requestId)),
              protocolVersion(std::move(version)),
              capabilities(std::move(caps)),
              clientInfo(std::move(info))
            {}

        std::string getMethod() const override { return "initialize"; }
        nlohmann::json getId() const override { return id; }
        std::optional<json> getParams() const override;
    };

    struct server_capabilities {
        bool resourceSubscriptions = false;
        bool toolSupport = false;
        bool promptTemplates = false;

        nlohmann::json toJson() const;
        static server_capabilities fromJson(const json& j);
    };

    struct server_info {
        std::string name;
        std::string version;

        nlohmann::json toJson() const;
        static server_info fromJson(const json& j);
    };

    class initialize_response : public response {
    private:
        std::string id;
        std::string protocolVersion;
        server_capabilities capabilities;
        server_info serverInfo;

    public:
        initialize_response(std::string responseId,
                                std::string version,
                                server_capabilities caps,
                                server_info info)

            : id(std::move(responseId)),
              protocolVersion(std::move(version)),
              capabilities(std::move(caps)),
              serverInfo(std::move(info))
            {}

        nlohmann::json getId() const override { return id; }
        std::optional<json> getResult() const override;
        static initialize_response fromJson(const json& j);
    };

    class initialized_notification : public notification {
    public:
        std::string getMethod() const override { return "initialized"; }
        std::optional<json> getParams() const override { return std::nullopt; }
    };
}

// ------------------------
// Usage Example
// ------------------------
/*
int main() {
    // Create client capabilities
    mcp::client_capabilities clientCaps;
    clientCaps.samplingSupport = true;
    clientCaps.notificationHandling = true;

    // Create client info
    mcp::client_info clientInfo;
    clientInfo.name = "MyMCPClient";
    clientInfo.version = "1.0.0";

    // Create initialize request
    mcp::initialize_request initReq("1", "1.0", clientCaps, clientInfo);

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
    mcp::initialize_response initResp = mcp::initialize_response::fromJson(serverRespJson);
    std::cout << "Server Info: " << initResp.toJson().dump(2) << "\n";

    return 0;
}
*/
