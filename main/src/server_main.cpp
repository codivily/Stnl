


#include "ticker.hpp"
#include "server_main.hpp"

#include "stnl/server.hpp"
#include "stnl/logger.hpp"
#include "stnl/request.hpp"

#include <boost/beast/version.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/asio.hpp>
#include <boost/system.hpp>
#include <boost/filesystem.hpp>
#include <boost/json.hpp>

#include <iostream>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <chrono>

namespace fs = boost::filesystem;
namespace http = beast::http;
namespace asio = boost::asio;
namespace json = boost::json;

using Logger = STNL::Logger;

ServerMain::ServerMain(std::shared_ptr<STNL::Server> server) : STNL::STNLModule(std::move(server)) {}

http::message_generator ServerMain::WebGetHome(const STNL::Request& req) {
  return STNL::Server::Response(req, GetServer()->GetRootDirPath() / "index.html", "text/html");
}

http::message_generator ServerMain::ApiPostData(const STNL::Request& req) {
  boost::json::object reqData = req.data();
  boost::json::object resData;
  std::shared_ptr<Ticker> ticker = GetServer()->GetModule<Ticker>();
  std::vector<std::string> fpaths;
  for(const STNL::UploadedFile& uf : req.files()) {
    fpaths.push_back(uf.file.string());
  }
  resData["data"] = reqData;
  resData["query"] = req.query();
  resData["status"] = true;
  resData["ticker"] = ticker->Increment();
  resData["files"] = json::value_from(fpaths);
  return STNL::Server::Response(req, resData);
}


void ServerMain::Setup() {
  Logger::Dbg("ServerMain::Setup()");
  GetServer()->Get("/", [this](const STNL::Request& req) -> http::message_generator { return this->WebGetHome(req); });
  GetServer()->Post("/api/data", [this](const STNL::Request& req) -> http::message_generator { return this->ApiPostData(req); });
}


void ServerMain::Launch() {
  Logger::Dbg("ServerMain::Launch()");
}