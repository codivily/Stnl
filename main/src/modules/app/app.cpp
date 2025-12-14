
#include "modules/app/app.hpp"
#include "modules/ticker/ticker.hpp"

#include "stnl/core/logger.hpp"
#include "stnl/db/blueprint.hpp"
#include "stnl/db/db.hpp"
#include "stnl/db/migration.hpp"
#include "stnl/http/request.hpp"
#include "stnl/http/server.hpp"

#include <boost/asio.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>
#include <boost/json.hpp>
#include <boost/system.hpp>

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>

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
using Migration = STNL::Migration;
using Blueprint = STNL::Blueprint;

App::App(Server &server) : STNLModule(server) {}

auto App::WebGetHome(Request const &req) -> http::message_generator {
    return Server::Response(req, server_.GetRootDirPath() / "index.html", "text/html");
}

auto App::ApiPostData(Request const &req) -> http::message_generator {
    boost::json::object reqData = req.data();
    boost::json::object resData;
    std::shared_ptr<Ticker> ticker = server_.GetModule<Ticker>();
    std::vector<std::string> fpaths;
    for (const UploadedFile &uf : req.files()) { fpaths.push_back(uf.file.string()); }
    resData["headers"] = json::value_from(req.headers());
    resData["data"] = reqData;
    resData["query"] = req.query();
    resData["status"] = true;
    resData["ticker"] = ticker->Increment();
    resData["files"] = json::value_from(fpaths);
    return Server::Response(req, resData);
}

auto App::ApiGetProduct(Request const &req) -> http::message_generator {
    std::shared_ptr<DB> pDB = server_.GetDatabase("default");
    boost::json::object reqData = req.data();
    boost::json::object queryData = req.query();
    QResult r = pDB->Exec("SELECT * FROM product LIMIT 10");
    boost::json::object resData;
    resData["result"] = r.ok;
    resData["data"] = pDB->ConvertPQXXResultToJson(r.data);
    return Server::Response(req, resData);
}

void App::SetupMigrations() {
    auto pDB = server_.GetDatabase();
    if (pDB) {
        pDB->GetMigration().Table("project", [](Blueprint &bp) {
            bp.BigInt("id").Identity().Index();
            bp.UUID("uuid").NotNull().Default().Unique();
            bp.Varchar("name").NotNull();
            bp.Bit("active").N(1).NotNull().Default();
        });
    }
}

void App::Setup() {
    Logger::Dbg() << ("App::Setup()");
    std::shared_ptr<App> self = std::static_pointer_cast<App>(shared_from_this());
    server_.Get("/", [self](Request const &req) { return self->WebGetHome(req); });
    server_.Post("/api/data", [self](Request const &req) { return self->ApiPostData(req); });
    server_.Get("/api/product", [self](Request const &req) { return self->ApiGetProduct(req); });
}

void App::Launch() {
    Logger::Dbg() << ("App::Launch()");
}
