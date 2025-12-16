#include "stnl/http/server.hpp"
#include "stnl/core/logger.hpp"
#include "stnl/core/stnl_module.hpp"
#include "stnl/db/db.hpp"
#include "stnl/db/migration.hpp"
#include "stnl/db/migrator.hpp"
#include "stnl/http/core.hpp"
#include "stnl/http/middleware.hpp"
#include "stnl/http/request.hpp"
#include "stnl/http/session.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace fs = boost::filesystem;

using tcp = boost::asio::ip::tcp;
namespace STNL {

Server::Server(asio::io_context &ioc, const tcp::endpoint &endpoint, fs::path rootDirPath)
    : ioc_(ioc), acceptor_(ioc, endpoint), rootDirPath_(std::move(std::move(rootDirPath))) {} // Fixed: acceptor needs ioc

void Server::AddDatabase(std::string const &keyAlias, std::string const &connectionString, size_t poolSize, size_t numThreads) {
    auto [it, inserted] = databases_.emplace(keyAlias, std::make_shared<DB>(connectionString, ioc_, poolSize, numThreads));
    if (inserted) { databaseKeyAliases_.emplace_back(keyAlias); }
}

auto Server::GetDatabase(std::string const &keyAlias) -> std::shared_ptr<DB> {
    auto it = databases_.find(keyAlias);
    if (it == databases_.end()) {
        Logger::Err() << std::format("Server::GetDatabase:: No database with "
                                     "this keyAlias found. KeyAlias: {}",
                                     keyAlias);
        return {nullptr};
    }
    return it->second;
}

auto Server::Response(Request const &req, const fs::path &file_path, const std::string &content_type, http::status status_code) -> http::message_generator {
    if (!fs::exists(file_path) || !fs::is_regular_file(file_path)) { return Server::Response(req, http::status::not_found); }
    http::response<http::file_body> res{status_code, req.GetHttpReq().version()};
    beast::error_code ec;
    res.body().open(file_path.string().c_str(), beast::file_mode::read, ec);
    if (ec) { return Server::Response(req, std::string("Internal Server Error"), http::status::internal_server_error); }
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, content_type);
    res.keep_alive(req.GetHttpReq().keep_alive());  // Respect client's keep-alive
    res.prepare_payload();
    return http::message_generator{std::move(res)};
}

auto Server::Response(Request const &req, http::status status_code) -> http::message_generator {
    http::response<http::empty_body> res{status_code, req.GetHttpReq().version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.prepare_payload();
    return http::message_generator{std::move(res)};
}

auto Server::Response(Request const &req, const std::string &msg, http::status status_code) -> http::message_generator {
    if (msg.empty()) {
        http::response<http::empty_body> res{status_code, req.GetHttpReq().version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.prepare_payload();
        return http::message_generator{std::move(res)};
    }
    http::response<http::string_body> res{status_code, req.GetHttpReq().version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/plain");
    res.body() = msg;
    res.prepare_payload();
    return http::message_generator{std::move(res)};
}

auto Server::Response(Request const &req, const boost::json::value &data, http::status status_code) -> http::message_generator {
    http::response<http::string_body> res{status_code, req.GetHttpReq().version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.body() = boost::json::serialize(data);
    res.prepare_payload();
    return http::message_generator{std::move(res)};
}

void Server::AddRoute(http::verb method, std::string path, RouteHandler handler) {
    router_.emplace(Route{method, std::move(path)}, std::move(handler));
}

void Server::Get(std::string path, RouteHandler handler) {
    AddRoute(http::verb::get, std::move(path),
             std::move(handler)); // Added move for efficiency
}

void Server::Post(std::string path, RouteHandler handler) {
    AddRoute(http::verb::post, std::move(path), std::move(handler));
}

void Server::Put(std::string path, RouteHandler handler) {
    AddRoute(http::verb::put, std::move(path), std::move(handler));
}

void Server::Delete(std::string path, RouteHandler handler) {
    AddRoute(http::verb::delete_, std::move(path),
             std::move(handler)); // Fixed: delete_ (keyword)
}

void Server::Options(std::string path, RouteHandler handler) {
    AddRoute(http::verb::options, std::move(path),
             std::move(handler)); // Fixed: options (not option)
}

auto Server::GetMiddlewares() const -> const std::vector<std::unique_ptr<Middleware>> & {
    return middlewares_;
}

auto Server::GetRouter() const -> const Router & {
    return router_;
}

void Server::DoAccept() {
    acceptor_.async_accept(asio::make_strand(ioc_), // Fixed: was beast::bind_front_handler in wrong place
                           beast::bind_front_handler(&Server::OnAccept, this));
}

void Server::OnAccept(beast::error_code ec, tcp::socket socket) {
    if (!ec) {
        std::make_shared<Session>(std::move(socket), *this)->Run();
    } else {
        Logger::Err() << "Server::OnAccept: " << ec.message();
    }
    DoAccept();
}

auto Server::GetRootDirPath() -> fs::path {
    return (rootDirPath_);
}

void Server::SetupModules() {
    for (const std::shared_ptr<STNLModule> &m : modulesVec_) {
        if (m) { m->Setup(); }
    }
}

void Server::SetupMiddlewares() {
    for (const std::unique_ptr<Middleware> &mw : middlewares_) {
        if (mw) { mw->Setup(); }
    }
}

void Server::RunDatabaseMigrations() {
    /* call the modules setupMigrations() method before runing the migrations
     * for each configured database */
    for (const std::shared_ptr<STNLModule> &m : modulesVec_) {
        if (m) { m->SetupMigrations(); }
    }
    for (std::string &dbKeyAlias : databaseKeyAliases_) {
        try {
            std::shared_ptr<DB> pDB = databases_.at(dbKeyAlias);
            Migrator::Migrate(*pDB);
        } catch (std::exception const &e) { Logger::Err() << "Server::RunDatabaseMigrations: " << std::string(e.what()); }
    }
}

void Server::LaunchModules() {
    for (const std::shared_ptr<STNLModule> &m : modulesVec_) {
        if (m) {
            asio::post(ioc_, [m]() {
                try {
                    m->Launch();
                } catch (std::exception &e) { Logger::Err() << "Server::LaunchModules: " << e.what(); }
            });
        }
    }
}

void Server::Run() {
    RunDatabaseMigrations();
    SetupModules();
    SetupMiddlewares();
    LaunchModules();
    DoAccept();
}

auto Server::GetIOC() -> asio::io_context & {
    return ioc_;
}

} // namespace STNL
