#include "stnl/server.hpp"
#include "stnl/core.hpp"
#include "stnl/db.hpp"
#include "stnl/stnl_module.hpp"
#include "stnl/session.hpp"
#include "stnl/request.hpp"
#include "stnl/middleware.hpp"
#include "stnl/logger.hpp"


#include <boost/beast/version.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <memory>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace fs = boost::filesystem;

using tcp = boost::asio::ip::tcp;
namespace STNL
{

    Server::Server(asio::io_context &ioc, tcp::endpoint endpoint, fs::path rootDirPath)
        : ioc_(ioc), acceptor_(ioc, endpoint), rootDirPath_(rootDirPath) {} // Fixed: acceptor needs ioc


    void Server::AddDatabase(std::string const& keyAlias, std::string& connectionString, size_t poolSize, size_t numThreads) {
        databases_.emplace(keyAlias, std::make_shared<DB>(connectionString, ioc_, poolSize, numThreads));
    }

    std::shared_ptr<DB> Server::GetDatabase(std::string const& keyAlias) {
        auto it = databases_.find(keyAlias);
        if (it == databases_.end()) {
            Logger::Err() << std::format("Server::GetDatabase:: No database with this keyAlias found. KeyAlias: {}", keyAlias);
            return std::shared_ptr<DB>(nullptr);
        }
        return it->second;
    }

    http::message_generator Server::Response(Request const& req, const fs::path& file_path, std::string content_type, http::status status_code) {
        if (!fs::exists(file_path) || !fs::is_regular_file(file_path)) {
            return Server::Response(req, http::status::not_found);
        }
        http::response<http::file_body> res{std::move(status_code), req.GetHttpReq().version()};
        beast::error_code ec;
        res.body().open(file_path.string().c_str(), beast::file_mode::read, ec);
        if (ec) {
           return Server::Response(req, std::string("Interval Server Error"), http::status::internal_server_error); 
        }
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, content_type);
        res.keep_alive(false);
        return http::message_generator{std::move(res)};
    }

    http::message_generator Server::Response(Request const& req, http::status status_code) {
        http::response<http::empty_body> res{std::move(status_code), req.GetHttpReq().version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.prepare_payload();
        return http::message_generator{std::move(res)};
    }
    
    http::message_generator Server::Response(Request const& req, const std::string& msg, http::status status_code) {
        if (msg.empty()) {
            http::response<http::empty_body> res{std::move(status_code), req.GetHttpReq().version()};
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

    http::message_generator Server::Response(Request const& req, const boost::json::value& data, http::status status_code) {
        http::response<http::string_body> res{std::move(status_code), req.GetHttpReq().version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.body() = boost::json::serialize(data);
        res.prepare_payload();
        return http::message_generator{std::move(res)};
    }

    void Server::AddRoute(http::verb method, std::string path, RouteHandler handler)
    {
        router_.emplace(Route{method, std::move(path)}, std::move(handler));
    }

    void Server::Get(std::string path, RouteHandler handler)
    {
        AddRoute(http::verb::get, std::move(path), std::move(handler)); // Added move for efficiency
    }

    void Server::Post(std::string path, RouteHandler handler)
    {
        AddRoute(http::verb::post, std::move(path), std::move(handler));
    }

    void Server::Put(std::string path, RouteHandler handler)
    {
        AddRoute(http::verb::put, std::move(path), std::move(handler));
    }

    void Server::Delete(std::string path, RouteHandler handler)
    {
        AddRoute(http::verb::delete_, std::move(path), std::move(handler)); // Fixed: delete_ (keyword)
    }

    void Server::Options(std::string path, RouteHandler handler)
    {
        AddRoute(http::verb::options, std::move(path), std::move(handler)); // Fixed: options (not option)
    }

    const std::vector<std::unique_ptr<Middleware>> &Server::GetMiddlewares() const
    {
        return middlewares_;
    }

    const Router &Server::GetRouter() const
    {
        return router_;
    }

    void Server::DoAccept()
    {
        acceptor_.async_accept(
            asio::make_strand(ioc_), // Fixed: was beast::bind_front_handler in wrong place
            beast::bind_front_handler(&Server::OnAccept, shared_from_this()));
    }

    void Server::OnAccept(beast::error_code ec, tcp::socket socket)
    {
        if (!ec)
        {
            std::make_shared<Session>(std::move(socket), shared_from_this())->Run();
        }
        else
        {
            std::cerr << "Server::OnAccept: ERROR: " << ec.message() << std::endl;
        }
        DoAccept();
    }

    fs::path Server::GetRootDirPath() { return (rootDirPath_); }

    void Server::SetupModules() {
        for (const std::shared_ptr<STNLModule>& m : modulesVec_) {
            if (m) { m->Setup(); }
        }   
    }

    void Server::SetupMiddlewares() {
        for (const std::unique_ptr<Middleware>& mw : middlewares_) {
            if (mw) { mw->Setup(); }
        }
    }

    void Server::LaunchModules() {
        for (const std::shared_ptr<STNLModule>& m : modulesVec_) {
            if (m) {
                asio::post(ioc_, [m]() {
                    try {
                        m->Launch();
                    } catch(std::exception& e) {
                        std::cerr << "Server::LaunchModules: Error: " << e.what() << std::endl;
                    }
                });
            }
        }
    }

    void Server::Run()
    {
        SetupModules();
        SetupMiddlewares();
        LaunchModules();
        DoAccept();
    }

    asio::io_context& Server::GetIOC() {
        return ioc_;
    }

} // namespace STNL