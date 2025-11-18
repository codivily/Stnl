


#include "ticker.hpp"
#include "server_main.hpp"

#include "stnl/server.hpp"
#include "stnl/logger.hpp"

#include <boost/beast/version.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/asio.hpp>
#include <boost/system.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <chrono>

namespace fs = boost::filesystem;
namespace http = beast::http;
namespace asio = boost::asio;

using Logger = STNL::Logger;

ServerMain::ServerMain(std::shared_ptr<STNL::Server> server) : STNL::STNLModule(std::move(server)) {}

http::message_generator ServerMain::webGetHome(const STNL::HttpRequest& req) {
  std::shared_ptr<Ticker> ticker = GetServer()->GetModule<Ticker>();
  int tickerValue = ticker->GetValue();
  ticker->Increment();
  if (ticker->GetValue() > 15) { ticker->Reset(); }
  // Logger::Dbg("Ticker::GetValue() -> " + std::to_string(tickerValue));

  if (tickerValue > 5) {
    std::string msg = "ServerMain:Ticker:Value: " + std::to_string(tickerValue);
    Logger::Dbg(msg);
    return STNL::Server::Response(req, http::status::ok, std::move(msg));
  }
  // const fs::path filePath = GetServer()->GetRootDirPath() / "index.html";
  // return STNL::Server::Response(req, std::move(filePath), "text/html");
  return STNL::Server::Response(req, GetServer()->GetRootDirPath() / "index.html", "text/html");
}


void ServerMain::Setup() {
  Logger::Dbg("ServerMain::Setup()");
  GetServer()->Get("/", [this](const STNL::HttpRequest& req) -> http::message_generator { return this->webGetHome(req); });
}

void ServerMain::Launch() {
  Logger::Dbg("ServerMain::Launch()");
}