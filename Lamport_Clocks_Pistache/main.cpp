#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/http.h>
#include <pistache/net.h>

#include <json/json.h>

#include <atomic>
#include <mutex>
#include <memory>
#include <iostream>

using namespace Pistache;

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

class LogicalClockServer {
public:
    explicit LogicalClockServer(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {}

    // void init(std::size_t threads = 4) {
    //     auto opts = Http::Endpoint::options()
    //                     .threads(static_cast<int>(threads))
    //                     .flags(Tcp::Options::InstallSignalHandler);

    //     httpEndpoint->init(opts);
    //     setupRoutes();
    // }
    void init(std::size_t threads = 4) {
        auto opts = Http::Endpoint::options()
                        .threads(static_cast<int>(threads));

        httpEndpoint->init(opts);
        setupRoutes();
    }


    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

    void shutdown() {
        httpEndpoint->shutdown();
    }

private:
    void setupRoutes() {
        using namespace Rest;

        Routes::Get(router, "/", Routes::bind(&LogicalClockServer::handleRoot, this));
        Routes::Get(router, "/time", Routes::bind(&LogicalClockServer::handleTime, this));
        Routes::Post(router, "/receive", Routes::bind(&LogicalClockServer::handleReceive, this));
        Routes::Get(router, "/send", Routes::bind(&LogicalClockServer::handleSend, this));
    }

    // Health check endpoint
    void handleRoot(const Rest::Request&, Http::ResponseWriter response) {
        response.send(Http::Code::Ok,
                      "Pistache server with Lamport Clock is running!\n");
    }

    // Endpoint to get current logical time
    void handleTime(const Rest::Request&, Http::ResponseWriter response) {
        int time = logicalClock.tick();

        Json::Value result;
        result["logical_time"] = time;
        result["message"] = "Local event occurred";

        Json::StreamWriterBuilder writer;
        std::string body = Json::writeString(writer, result);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, body);
    }

    // Endpoint to receive a message with a timestamp
    void handleReceive(const Rest::Request& request, Http::ResponseWriter response) {
        const auto& body = request.body();
        if (body.empty()) {
            response.send(Http::Code::Bad_Request,
                          "Missing JSON body\n");
            return;
        }

        Json::CharReaderBuilder rbuilder;
        std::unique_ptr<Json::CharReader> reader(rbuilder.newCharReader());

        Json::Value json;
        std::string errs;

        bool ok = reader->parse(body.c_str(), body.c_str() + body.size(), &json, &errs);
        if (!ok || !json.isMember("timestamp") || !json["timestamp"].isInt()) {
            response.send(Http::Code::Bad_Request,
                          "Missing or invalid 'timestamp' in request body\n");
            return;
        }

        int received_time = json["timestamp"].asInt();
        int updated_time = logicalClock.update(received_time);

        Json::Value result;
        result["received_timestamp"] = received_time;
        result["updated_logical_time"] = updated_time;
        result["message"] = "Clock synchronized with received event";

        Json::StreamWriterBuilder writer;
        std::string respBody = Json::writeString(writer, result);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, respBody);
    }

    // Endpoint to send a message
    void handleSend(const Rest::Request&, Http::ResponseWriter response) {
        int time = logicalClock.tick();

        Json::Value result;
        result["logical_time"] = time;
        result["message"] = "Use this timestamp when sending to another node";

        Json::StreamWriterBuilder writer;
        std::string body = Json::writeString(writer, result);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, body);
    }

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main() {
    Port port(8080);
    Address addr(Ipv4::any(), port);

    std::cout << "Server starting on port 8080" << std::endl;

    LogicalClockServer server(addr);
    server.init(4);
    server.start();

    return 0;
}
