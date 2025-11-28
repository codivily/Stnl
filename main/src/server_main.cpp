


#include "ticker.hpp"
#include "server_main.hpp"

#include "stnl/server.hpp"
#include "stnl/logger.hpp"
#include "stnl/request.hpp"
#include "stnl/db.hpp"

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
using Server = STNL::Server;
using Request = STNL::Request;
using STNLModule = STNL::STNLModule;
using UploadedFile = STNL::UploadedFile;
using DB = STNL::DB;
using QResult = STNL::QResult;

ServerMain::ServerMain(Server& server) : STNLModule(server) {}

http::message_generator ServerMain::WebGetHome(Request const& req) {
  return Server::Response(req, server_.GetRootDirPath() / "index.html", "text/html");
}

http::message_generator ServerMain::ApiPostData(Request const& req) {
  boost::json::object reqData = req.data();
  boost::json::object resData;
  std::shared_ptr<Ticker> ticker = server_.GetModule<Ticker>();
  std::vector<std::string> fpaths;
  for(const UploadedFile& uf : req.files()) {
    fpaths.push_back(uf.file.string());
  }
  resData["headers"] = json::value_from(req.headers());
  resData["data"] = reqData;
  resData["query"] = req.query();
  resData["status"] = true;
  resData["ticker"] = ticker->Increment();
  resData["files"] = json::value_from(fpaths);
  return Server::Response(req, resData);
}


http::message_generator ServerMain::ApiGetProduct(Request const& req) {
  std::shared_ptr<DB> pDB = server_.GetDatabase("default");
  boost::json::object reqData = req.data();
  boost::json::object queryData = req.query();
  QResult r = pDB->Exec("SELECT * FROM product LIMIT 10");
  boost::json::object resData;
  resData["result"] = r.ok;
  resData["data"] = pDB->ConvertPQXXResultToJson(r.data);
  return Server::Response(req, resData);
}


void ServerMain::Setup() {
  Logger::Dbg() << ("ServerMain::Setup()");
  std::shared_ptr<ServerMain> self = shared_from_this();
  server_.Get("/", [self](Request const& req) { return self->WebGetHome(req); });
  server_.Post("/api/data", [self](Request const& req) { return self->ApiPostData(req); });
  server_.Get("/api/product", [self](Request const& req) { return self->ApiGetProduct(req); });
}


void ServerMain::Launch() {
  Logger::Dbg() << ("ServerMain::Launch()");
}