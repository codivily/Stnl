
#include "server_main.hpp"
#include "ticker.hpp"

#include "stnl/server.hpp"
#include "stnl/logger.hpp"
#include "stnl/middleware.hpp"
#include "stnl/config.hpp"
#include "stnl/db.hpp"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <thread>


namespace fs = boost::filesystem;
namespace asio = boost::asio;
namespace http = boost::beast::http;

using Logger = STNL::Logger;
using Config = STNL::Config;
using Server = STNL::Server;
using Request = STNL::Request;
using Middleware = STNL::Middleware;
using DB = STNL::DB;

 class BasicMiddleware : public Middleware {
  public:
    BasicMiddleware(Server& server) : Middleware(server) {}
    boost::optional<http::message_generator> invoke(Request& req) override {
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
        Config::Init(rootDirPath / "config.local.json");
    } 
    else if (fs::exists(rootDirPath / "config.json")) {
        Config::Init(rootDirPath / "config.json");
    } 
    else {
        Config::Init(rootDirPath / "config.json");
    }

    // Setup the database connection
    auto dbName = Config::Value<std::string>("database.name", std::string("stnl_db"));
    auto dbUser = Config::Value<std::string>("database.user", std::string("postgres"));
    auto dbPassword = Config::Value<std::string>("database.password", std::string("!stnl1301"));
    auto dbHost = Config::Value<std::string>("database.host", std::string("localhost"));
    auto dbPort = Config::Value<int>("database.port", 5432);
    auto dbSchema = Config::Value<std::string>("database.schema", std::string("public"));

    std::string connStr = DB::GetConnectionString(*dbName, *dbUser, *dbPassword, *dbHost, *dbPort, *dbSchema);

    boost::optional<std::string> serverHost = Config::Value<std::string>("server.host", boost::optional<std::string>(std::string("127.0.0.1")));
    boost::optional<int> serverPort = Config::Value<int>("server.port", boost::optional<int>(8080));
    if (serverHost) { endpoint.address(asio::ip::make_address(serverHost.value())); }
    if (serverPort) { endpoint.port(serverPort.value()); }
    Logger::Inf() << ("::main:: Server listening on " + endpoint.address().to_string() + ":" + std::to_string(endpoint.port()));

    Server server{ioc, endpoint, rootDirPath};
    
    server.Use<BasicMiddleware>();
    server.AddDatabase("default", connStr);
    
    server.AddModule<ServerMain>();
    server.AddModule<Ticker>();
    server.Run();

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