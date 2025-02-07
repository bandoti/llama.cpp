#include <thread>
#include <atomic>
#include <iostream>

class StdioTransport : public ITransport {
public:
    StdioTransport() : running(false) {}

    bool start() override {
        running = true;
        // Start a thread to read from stdin
        readThread = std::thread(&StdioTransport::readLoop, this);
        return true;
    }

    void stop() override {
        running = false;
        if (readThread.joinable()) {
            readThread.join();
        }
    }

    bool sendMessage(const std::string& message) override {
        std::cout << message << std::endl;
        return true;
    }

    void setMessageHandler(const std::function<void(const std::string&)>& handler) override {
        messageHandler = handler;
    }

private:
    void readLoop() {
        std::string line;
        while (running && std::getline(std::cin, line)) {
            if (messageHandler) {
                messageHandler(line);
            }
        }
    }

    std::atomic<bool> running;
    std::thread readThread;
    std::function<void(const std::string&)> messageHandler;
};
