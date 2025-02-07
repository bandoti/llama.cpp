#include <string>
#include <functional>

class ITransport {
public:
    virtual ~ITransport() = default;

    // Starts the transport mechanism
    virtual bool start() = 0;

    // Stops the transport mechanism
    virtual void stop() = 0;

    // Sends a message through the transport
    virtual bool sendMessage(const std::string& message) = 0;

    // Sets a callback function to handle received messages
    virtual void setMessageHandler(const std::function<void(const std::string&)>& handler) = 0;
};

