
#include "modules/app/app.hpp"
#include "modules/ticker/ticker.hpp"

#include "stnl/core/config.hpp"
#include "stnl/core/logger.hpp"
#include "stnl/db/blueprint.hpp"
#include "stnl/db/db.hpp"
#include "stnl/db/sr_blueprint.hpp"
#include "stnl/http/middleware.hpp"
#include "stnl/http/server.hpp"

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>

namespace fs = boost::filesystem;
namespace asio = boost::asio;
namespace http = boost::beast::http;

using Logger = STNL::Logger;
using Config = STNL::Config;
using Server = STNL::Server;
using Request = STNL::Request;
using Middleware = STNL::Middleware;
using DB = STNL::DB;
using Blueprint = STNL::Blueprint;
using SrBlueprint = STNL::SrBlueprint;

class BasicMiddleware : public Middleware {
  public:
    BasicMiddleware(Server &server) : Middleware(server) {}
    auto invoke(Request &req) -> boost::optional<http::message_generator> override {
        Logger::Inf() << ("BasicMiddleware: Request for " + std::string(req.GetHttpReq().target()));
        return boost::none; // Continue processing
    }
    void Setup() override { Logger::Inf() << ("BasicMiddleware::Setup()"); }
};

auto main(int /*argc*/, char ** /*argv*/) -> int {
    asio::io_context ioc;

    static constexpr int DEFAULT_SERVER_PORT = 8080;
    static constexpr int DEFAULT_DB_PORT = 5432;
    tcp::endpoint endpoint{tcp::v4(), DEFAULT_SERVER_PORT};

    // Use current working directory as the server root directory per user
    // request.
    fs::path rootDirPath = fs::current_path();
    try {
        rootDirPath = fs::canonical(rootDirPath);
    } catch (const std::exception &) { /* fallback to current_path() */
    }
    Logger::Inf() << ("::main:: Server's rootDirPath: " + rootDirPath.string());

    if (fs::exists(rootDirPath / "config.local.json")) {
        Config::Init(rootDirPath / "config.local.json");
    } else if (fs::exists(rootDirPath / "config.json")) {
        Config::Init(rootDirPath / "config.json");
    } else {
        Config::Init(rootDirPath / "config.json");
    }

    // Setup the database connection
    // Keep these on single lines for readability
    // clang-format off
    auto dbName = Config::Value<std::string>("database.name", std::string("postgres"));
    auto dbUser = Config::Value<std::string>("database.user", std::string("postgres"));
    auto dbPassword = Config::Value<std::string>("database.password", std::string("postgres"));
    auto dbHost = Config::Value<std::string>("database.host", std::string("localhost"));
    auto dbPort = Config::Value<int>("database.port", DEFAULT_DB_PORT);
    auto dbSchema = Config::Value<std::string>("database.schema", std::string("public"));
    // clang-format on
    boost::optional<std::string> serverHost = Config::Value<std::string>("server.host", boost::optional<std::string>(std::string("127.0.0.1")));
    boost::optional<int> serverPort = Config::Value<int>("server.port", boost::optional<int>(DEFAULT_SERVER_PORT));
    if (serverHost) { endpoint.address(asio::ip::make_address(serverHost.value())); }
    if (serverPort) { endpoint.port(serverPort.value()); }
    Logger::Inf() << ("::main:: Server listening on " + endpoint.address().to_string() + ":" + std::to_string(endpoint.port()));

    Server server{ioc, endpoint, rootDirPath};

    server.Use<BasicMiddleware>();
    std::string connStr = DB::GetConnectionString(*dbName, *dbUser, *dbPassword, *dbHost, *dbPort, *dbSchema);
    server.AddDatabase("default", connStr);

    // add migration to the database that will later run when the server starts
    auto pDB = server.GetDatabase();
    pDB->GetMigration().Table("asset", [](Blueprint &bp) {
        bp.BigInt("id").Identity().Index();
        bp.UUID("uuid").NotNull().Default().Unique();
        bp.Varchar("name").NotNull();
        bp.Bit("active").N(1).NotNull().Default();
    });

    pDB->GetMigration().Procedure("sr_test", [](SrBlueprint &bp) {
        bp.Body() = R"(
            BEGIN
                SELECT * FROM product ORDER BY uuid desc LIMIT 10;
            END;
        )";
    });

    server.AddModule<App>();
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
    for (std::thread &t : threads) { t.join(); }
}