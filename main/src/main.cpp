
#include "server_main.hpp"
#include "ticker.hpp"
#include "database.hpp"

#include "stnl/server.hpp"
#include "stnl/logger.hpp"
#include "stnl/middleware.hpp"
#include "stnl/config.hpp"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <thread>


namespace fs = boost::filesystem;
namespace asio = boost::asio;
namespace http = boost::beast::http;

using Logger = STNL::Logger;

 class BasicMiddleware : public STNL::Middleware {
  public:
    BasicMiddleware(std::shared_ptr<STNL::Server> server) : STNL::Middleware(std::move(server)) {}
    boost::optional<http::message_generator> invoke(STNL::Request& req) override {
        Logger::Inf() << ("BasicMiddleware: Request for " + std::string(req.GetHttpReq().target()));
        return boost::none; // Continue processing
    }

    void Setup() override {
        Logger::Inf() << ("BasicMiddleware::Setup()");
    }
 };

int main(int argc, char* argv[]) {
    asio::io_context ioc;
    
    tcp::endpoint endpoint{tcp::v4(), 8080};
    
    fs::path rootDirPath(argv[0]);
    rootDirPath = fs::canonical(rootDirPath);
    rootDirPath = rootDirPath.parent_path();
    Logger::Inf() << ("::main:: Server's rootDirPath: " + rootDirPath.string());
    
    if (fs::exists(rootDirPath / "config.local.json")) {
        STNL::Config::Init(rootDirPath / "config.local.json");
    } 
    else if (fs::exists(rootDirPath / "config.json")) {
        STNL::Config::Init(rootDirPath / "config.json");
    } 
    else {
        STNL::Config::Init(rootDirPath / "config.json");
    }

    boost::optional<std::string> serverHost = STNL::Config::Value<std::string>("server.host", boost::optional<std::string>("127.0.0.1"));
    boost::optional<int> serverPort = STNL::Config::Value<int>("server.port", boost::optional<int>(8080));
    if (serverHost) { endpoint.address(asio::ip::make_address(serverHost.value())); }
    if (serverPort) { endpoint.port(serverPort.value()); }
    Logger::Inf() << ("::main:: Server listening on " + endpoint.address().to_string() + ":" + std::to_string(endpoint.port()));

    std::shared_ptr<STNL::Server> server = std::make_shared<STNL::Server>(ioc, endpoint, rootDirPath);
    
    server->Use<BasicMiddleware>();
    
    server->AddModule<Database>();
    server->AddModule<ServerMain>();
    server->AddModule<Ticker>();
    server->Run();

    // Get the max threads = the number of logical cores
    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) { numThreads = 1; }
    
    // init Logger io context before starting threads
    Logger::Init(ioc);

    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (unsigned int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&ioc] { ioc.run(); });
    }
    /* Wait for threads **/
    for (std::thread& t : threads) { t.join(); }
    
    return 0;
}