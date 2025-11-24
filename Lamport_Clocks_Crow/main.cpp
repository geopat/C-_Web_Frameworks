#include "crow.h"
#include <mutex>
#include <atomic>
#include <sstream>

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

int main() {
    crow::SimpleApp app;
    LamportClock clock;

    // Health check endpoint
    CROW_ROUTE(app, "/")
    ([]() {
        return "Crow server with Lamport Clock is running!";
    });

    // Endpoint to get current logical time
    CROW_ROUTE(app, "/time")
    ([&clock]() {
        int time = clock.tick();
        crow::json::wvalue result;
        result["logical_time"] = time;
        result["message"] = "Local event occurred";
        return result;
    });

    // Endpoint to receive a message with a timestamp (simulates distributed event)
    CROW_ROUTE(app, "/receive")
        .methods("POST"_method)
    ([&clock](const crow::request& req) {
        auto body = crow::json::load(req.body);
        
        if (!body || !body.has("timestamp")) {
            return crow::response(400, "Missing timestamp in request body");
        }

        int received_time = body["timestamp"].i();
        int updated_time = clock.update(received_time);

        crow::json::wvalue result;
        result["received_timestamp"] = received_time;
        result["updated_logical_time"] = updated_time;
        result["message"] = "Clock synchronized with received event";
        
        return crow::response(result);
    });

    // Endpoint to send a message (gets current time to send to another node)
    CROW_ROUTE(app, "/send")
    ([&clock]() {
        int time = clock.tick();
        crow::json::wvalue result;
        result["logical_time"] = time;
        result["message"] = "Use this timestamp when sending to another node";
        return result;
    });

    app.port(8080).multithreaded().run();
}