#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <curl/curl.h>

class HttpSseTransport : public ITransport {
public:
    HttpSseTransport(const std::string& sseUrl, const std::string& postUrl)
        : sseUrl(sseUrl), postUrl(postUrl), running(false) {}

    bool start() override {
        running = true;
        // Start a thread to handle SSE connection
        sseThread = std::thread(&HttpSseTransport::sseLoop, this);
        return true;
    }

    void stop() override {
        running = false;
        if (sseThread.joinable()) {
            sseThread.join();
        }
    }

    bool sendMessage(const std::string& message) override {
        // Use libcurl to send HTTP POST request
        CURL* curl = curl_easy_init();
        if (!curl) return false;

        curl_easy_setopt(curl, CURLOPT_URL, postUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message.c_str());

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        return (res == CURLE_OK);
    }

    void setMessageHandler(const std::function<void(const std::string&)>& handler) override {
        messageHandler = handler;
    }

private:
    void sseLoop() {
        // Use libcurl to establish SSE connection
        CURL* curl = curl_easy_init();
        if (!curl) return;

        curl_easy_setopt(curl, CURLOPT_URL, sseUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &HttpSseTransport::writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

        while (running) {
            curl_easy_perform(curl);
            // Handle reconnection logic if needed
        }

        curl_easy_cleanup(curl);
    }

    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        size_t totalSize = size * nmemb;
        HttpSseTransport* self = static_cast<HttpSseTransport*>(userdata);
        if (self->messageHandler) {
            self->messageHandler(std::string(ptr, totalSize));
        }
        return totalSize;
    }

    std::string sseUrl;
    std::string postUrl;
    std::atomic<bool> running;
    std::thread sseThread;
    std::function<void(const std::string&)> messageHandler;
};
