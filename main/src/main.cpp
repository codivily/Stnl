#include "stnl/server.hpp"
#include "stnl/logger.hpp"

#include "server_main.hpp"
#include "ticker.hpp"
#include "database.hpp"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <thread>


namespace fs = boost::filesystem;
namespace asio = boost::asio;
namespace http = boost::beast::http;

using Logger = STNL::Logger;

int main(int argc, char* argv[]) {
    asio::io_context ioc;
    Logger::Init(ioc);

    tcp::endpoint endpoint{tcp::v4(), 8080};

    fs::path rootDirPath(argv[0]);
    rootDirPath = fs::canonical(rootDirPath);
    rootDirPath = rootDirPath.parent_path();
    Logger::Inf("::main:: Server's rootDirPath: " + rootDirPath.string());

    std::shared_ptr<STNL::Server> server = std::make_shared<STNL::Server>(ioc, endpoint, rootDirPath);
    server->UseMiddleware([](STNL::Request& req) -> boost::optional<http::message_generator> {
        Logger::Inf("AuthMiddleware: Incoming request for " + std::string(req.GetHttpReq().target()));
        //return boost::optional<http::message_generator>{std::move(STNL::Server::Response(req, "Unauthorized", http::status::unauthorized))};
        return boost::none;
    });
    server->AddModule<Database>();
    server->AddModule<ServerMain>();
    server->AddModule<Ticker>();
    server->Run();

    // Get the max threads = the number of logical cores
    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) { numThreads = 1; }
    
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (unsigned int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&ioc] { ioc.run(); });
    }

    Logger::Inf("::main::  Server started on port 8080 with " + std::to_string(numThreads) + " threads.");
    
    /* Wait for threads **/
    for (std::thread& t : threads) { t.join(); }
    
    return 0;
}