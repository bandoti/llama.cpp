#include <string>
#include <optional>
#include <vector>
#include <nlohmann/json.hpp>

namespace mcp
{
    extern const std::string JsonRpcVersion;
    extern const std::string McpVersion;
    extern const std::string ClientVersion;
    extern const std::string ClientName;

    class message {
    public:
        message(std::optional<nlohmann::json> id = std::nullopt);

        virtual ~message() = default;
        virtual nlohmann::json toJson() const = 0;

        void id(std::optional<nlohmann::json> id);
        const std::optional<nlohmann::json> & id() const;

    private:
        std::optional<nlohmann::json> id_;
    };


    class request : public message {
    public:
        request(std::optional<nlohmann::json> id,
                std::string method,
                std::optional<nlohmann::json> params = std::nullopt);

        virtual ~request() = default;
        nlohmann::json toJson() const override;

        void method(std::string method);
        const std::string & method() const;

        void params(std::optional<nlohmann::json> params);
        const std::optional<nlohmann::json> & params() const;

    private:
        std::string method_;
        std::optional<nlohmann::json> params_;
    };


    class response : public message {
    public:
        struct error {
            int code;
            std::string message;
            std::optional<nlohmann::json> data;
            nlohmann::json toJson() const;
        };

        response(std::optional<nlohmann::json> id,
                 std::optional<nlohmann::json> result = std::nullopt,
                 std::optional<error> error = std::nullopt);

        virtual ~response() = default;
        virtual nlohmann::json toJson() const override;

        void result(std::optional<nlohmann::json> result);
        const std::optional<nlohmann::json> & result() const;

        void error(std::optional<error> error);
        const std::optional<error> & error() const;

    private:
        std::optional<nlohmann::json> result_;
        std::optional<error> error_;
    };


    class notification : public message {
    public:
        notification(std::string method,
                     std::optional<nlohmann::json> params = std::nullopt);

        virtual ~notification() = default;
        virtual nlohmann::json toJson() const override;

        void method(std::string method);
        const std::string & method() const;

        void params(std::optional<nlohmann::json> params);
        const std::optional<nlohmann::json> & params() const;

    private:
        std::string method_;
        std::optional<nlohmann::json> params_;
    };


    struct capability {
        std::string name;
        bool subscribe   = false;
        bool listChanged = false;
    };

    using capabilities = std::vector<capability>;

    class initialize_request : public request {
    public:
        initialize_request(nlohmann::json id, capabilities caps);

        const std::string & name()    const { return ClientName; }
        const std::string & version() const { return ClientVersion; }
        const std::string & protoVersion() const { return McpVersion; }

        void capabilities(capabilities capabilities);
        const capabilities & capabilities() const;

    private:
        void refreshParams();

        capabilities caps_;
    };


    class initialize_response : public response {
    public:
        initialize_response(nlohmann::json id,
                            std::string name,
                            std::string version,
                            std::string protoVersion,
                            capabilities caps);

        void name(std::string name);
        const std::string & name() const;

        void version(std::String version);
        const std::string & version() const;

        void protoVersion(std::string protoVersion);
        const std::string & protoVersion() const;

        void capabilities(capabilities capabilities);
        const capabilities & capabilities() const;

        static initialize_response fromJson(const json& j);

    private:
        void refreshResult();

        std::string name_;
        std::string version_;
        std::string protoVersion_;
        capabilities caps_;
    };


    class initialized_notification : public notification {
    public:
        initialized_notification();
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
