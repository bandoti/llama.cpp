#include <iostream>

int main() {
    // Example usage with StdioTransport
    StdioTransport stdioTransport;
    stdioTransport.setMessageHandler([](const std::string& message) {
        std::cout << "Received: " << message << std::endl;
    });

    if (stdioTransport.start()) {
        stdioTransport.sendMessage("Hello, MCP!");
        // Perform other operations
        stdioTransport.stop();
    }

    // Example usage with HttpSseTransport
    HttpSseTransport httpSseTransport("http://server/sse", "http://server/post");
    httpSseTransport.setMessageHandler([](const std::string& message) {
        std::cout << "Received via SSE: " << message << std::endl;
    });

    if (httpSseTransport.start()) {
        httpSseTransport.sendMessage("Hello, MCP over HTTP!");
        // Perform other operations
        httpSseTransport.stop();
    }

    return 0;
}
