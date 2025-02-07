#include <string>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class IMessage {
public:
    virtual ~IMessage() = default;

    virtual std::string getJsonRpcVersion() const = 0;
    virtual std::optional<std::string> getMethod() const = 0;
    virtual std::optional<json> getId() const = 0;
    virtual std::optional<json> getParams() const = 0;

    virtual json toJson() const = 0;
};

class IRequest : public IMessage {
public:
    virtual ~IRequest() = default;

    virtual std::string getMethod() const override = 0;
    virtual json getId() const override = 0;
    virtual std::optional<json> getParams() const override = 0;

    json toJson() const override {
        json j;
        j["jsonrpc"] = "2.0";
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

    virtual json getId() const override = 0;
    virtual std::optional<json> getResult() const = 0;
    virtual std::optional<Error> getError() const = 0;

    json toJson() const override {
        json j;
        j["jsonrpc"] = "2.0";
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

    virtual std::string getMethod() const override = 0;
    virtual std::optional<json> getParams() const override = 0;

    json toJson() const override {
        json j;
        j["jsonrpc"] = "2.0";
        j["method"] = getMethod();
        if (getParams()) {
            j["params"] = getParams().value();
        }
        return j;
    }
};
