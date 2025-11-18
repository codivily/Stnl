


#include "ticker.hpp"
#include "server_main.hpp"

#include "stnl/server.hpp"
#include "stnl/utils.hpp"

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

ServerMain::ServerMain(std::shared_ptr<STNL::Server> server) : STNL::STNLModule(std::move(server)) {}

http::message_generator ServerMain::webGetHome(const STNL::HttpRequest& req) {
  std::shared_ptr<Ticker> ticker = GetServer()->GetModule<Ticker>();
  int tickerValue = ticker->GetValue();
  ticker->Increment();
  if (ticker->GetValue() > 15) { ticker->Reset(); }
  // Logger::Dbg("Ticker::GetValue() -> " + std::to_string(tickerValue));

  if (tickerValue > 5) {
    std::string msg = "Ticker: " + std::to_string(tickerValue);
    return STNL::Server::Response(req, http::status::ok, msg);
  }
  // const fs::path filePath = GetServer()->GetRootDirPath() / "index.html";
  // return STNL::Server::Response(req, std::move(filePath), "text/html");
  return STNL::Server::Response(req, GetServer()->GetRootDirPath() / "index.html", "text/html");
}


void ServerMain::Setup() {
  GetServer()->Get("/", [this](const STNL::HttpRequest& req) -> http::message_generator { return this->webGetHome(req); });
  std::cout << "ServerMain::Setup()" << std::endl;
}

void ServerMain::Launch() {
  std::cout << "ServerMain::Launch()" << std::endl;
}