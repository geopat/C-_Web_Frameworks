#include <drogon/drogon.h>
#include <mutex>
#include <atomic>

using namespace drogon;

// Lamport Logical Clock implementation
class LamportClock {
private:
    std::atomic<int> counter{0};
    std::mutex mtx;

public:
    int tick() {
        return ++counter;
    }

    int update(int received_time) {
        std::lock_guard<std::mutex> lock(mtx);
        int current = counter.load();
        counter = std::max(current, received_time) + 1;
        return counter;
    }

    int get() const {
        return counter.load();
    }
};

// Global clock instance
LamportClock logicalClock;

int main() {
    // Health check endpoint
    app().registerHandler("/",
        [](const HttpRequestPtr& req,
           std::function<void(const HttpResponsePtr&)>&& callback) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setBody("Drogon server with Lamport Clock is running!");
            callback(resp);
        });

    // Endpoint to get current logical time
    app().registerHandler("/time",
        [](const HttpRequestPtr& req,
           std::function<void(const HttpResponsePtr&)>&& callback) {
            int time = logicalClock.tick();
            
            Json::Value result;
            result["logical_time"] = time;
            result["message"] = "Local event occurred";
            
            auto resp = HttpResponse::newHttpJsonResponse(result);
            callback(resp);
        });

    // Endpoint to receive a message with a timestamp
    app().registerHandler("/receive",
        [](const HttpRequestPtr& req,
           std::function<void(const HttpResponsePtr&)>&& callback) {
            auto json = req->getJsonObject();
            
            if (!json || !json->isMember("timestamp")) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k400BadRequest);
                resp->setBody("Missing timestamp in request body");
                callback(resp);
                return;
            }

            int received_time = (*json)["timestamp"].asInt();
            int updated_time = logicalClock.update(received_time);

            Json::Value result;
            result["received_timestamp"] = received_time;
            result["updated_logical_time"] = updated_time;
            result["message"] = "Clock synchronized with received event";
            
            auto resp = HttpResponse::newHttpJsonResponse(result);
            callback(resp);
        },
        {Post});

    // Endpoint to send a message
    app().registerHandler("/send",
        [](const HttpRequestPtr& req,
           std::function<void(const HttpResponsePtr&)>&& callback) {
            int time = logicalClock.tick();
            
            Json::Value result;
            result["logical_time"] = time;
            result["message"] = "Use this timestamp when sending to another node";
            
            auto resp = HttpResponse::newHttpJsonResponse(result);
            callback(resp);
        });

    // Configure and start server
    app().addListener("0.0.0.0", 8080);
    app().setThreadNum(4);
    
    LOG_INFO << "Server starting on port 8080";
    app().run();
    
    return 0;
}