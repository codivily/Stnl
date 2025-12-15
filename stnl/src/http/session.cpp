#include "stnl/http/session.hpp"
#include "stnl/core/logger.hpp"
#include "stnl/http/core.hpp"
#include "stnl/http/middleware.hpp"
#include "stnl/http/request.hpp"
#include "stnl/http/server.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/read.hpp>  // For async_read
#include <boost/beast/http/write.hpp> // For async_write
#include <boost/beast/version.hpp>    // For BOOST_BEAST_VERSION_STRING
#include <boost/filesystem.hpp>

#include <algorithm>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace fs = boost::filesystem;
using tcp = boost::asio::ip::tcp;

namespace STNL {

Session::Session(tcp::socket socket, Server &server) : stream_(std::move(socket)), server_(server), keepAlive_(false) {}

void Session::Run() {
    DoRead();
}

void Session::DoRead() {
    httpReq_ = HttpRequest{};
    http::async_read(stream_, buffer_, httpReq_, beast::bind_front_handler(&Session::OnRead, shared_from_this()));
}

void Session::OnRead(beast::error_code ec, std::size_t /*bytes_transferred*/) {
    if (ec == http::error::end_of_stream) {
        beast::error_code ec2;
        stream_.socket().shutdown(tcp::socket::shutdown_send,
                                  ec2); // Fixed: was "shutdown_end"
        return;
    }
    if (ec) {
        std::cerr << "Session::OnRead: ERROR: " << ec.message() << '\n';
        return;
    }
    Request req = Request::parse(httpReq_); // Wrap the HttpRequest for easier access
    http::message_generator res = HandleRequest(req);
    keepAlive_ = res.keep_alive();
    beast::async_write(stream_, std::move(res), beast::bind_front_handler(&Session::OnWrite, shared_from_this()));
}

void Session::OnWrite(beast::error_code ec, std::size_t /*bytes_transferred*/) {
    if (ec) {
        std::cerr << "Session::OnWrite: ERROR: " << ec.message() << '\n';
        return;
    }
    if (!keepAlive_) { // Fixed: was "need_eof()" which isn't standard; use
                       // keep_alive check
        beast::error_code ec2;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec2);
    } else {
        DoRead();
    }
}

auto Session::ApplyMiddlewares(Request &req) -> boost::optional<http::message_generator> {
    for (const std::unique_ptr<Middleware> &mdw : server_.GetMiddlewares()) {
        boost::optional<http::message_generator> result = mdw->invoke(req);
        if (result.has_value()) { return {std::move(result.value())}; }
    }
    return boost::none;
}

static auto _GetFileMimeType(fs::path const& filePath) -> std::string {
    // std::string ext = publicPath.extension().string();
    std::string ext = filePath.extension().string();
    if (ext == ".html") { return "text/html"; }
    if (ext == ".css") { return "text/css"; }
    if (ext == ".js") { return "application/javascript"; }
    if (ext == ".png") { return "image/png"; }
    if (ext == ".jpg") { return "image/jpg"; }
    if (ext == ".jpeg" ) { return "image/jpeg"; }
    Logger::Dbg() << "GetFileMimeType: Unknown file extension:" + filePath.extension().string();
    return "application/octet-stream";
}

static bool _IsLexicalSubpath(const fs::path& base_path, const fs::path& child_path) {
    const fs::path base_norm = base_path.lexically_normal();
    const fs::path child_norm = child_path.lexically_normal();
    auto [base_mismatch, child_mismatch] = std::mismatch(
        base_norm.begin(), base_norm.end(),
        child_norm.begin()
    );
    return base_mismatch == base_norm.end();
}

auto Session::HandleRequest(Request &req) -> http::message_generator {
    // FIXED: Extract path-only (strip query) for routing
    std::string_view full_target{httpReq_.target()};
    std::string_view path = full_target;
    size_t query_pos = full_target.find('?');
    if (query_pos != std::string_view::npos) { path = full_target.substr(0, query_pos); }
    Route key{httpReq_.method(), std::string(path)}; // Use path-only
    auto it = server_.GetRouter().find(key); 
    if (it == server_.GetRouter().end()) {
        if (path.starts_with("/api/") || path == "/api") {
            return Server::Response(req, http::status::not_found);
        }
        if (httpReq_.method() == http::verb::get) {
            fs::path publicDirPath = server_.GetRootDirPath() / "public";
            if (fs::exists(publicDirPath)) {
                fs::path targetFilePath = publicDirPath / path;
                if (!fs::exists(targetFilePath) || !fs::is_regular_file(targetFilePath) || !_IsLexicalSubpath(publicDirPath, targetFilePath)) {
                    targetFilePath = publicDirPath / "index.html";
                }
                if (fs::exists(targetFilePath)) {
                    return Server::Response(req, targetFilePath, _GetFileMimeType(targetFilePath));
                }
            }
            // TODO: check in the project public directory
        }
        return Server::Response(req, http::status::not_found);
    }
    // Run the middleware chain before handler
    boost::optional<http::message_generator> result = ApplyMiddlewares(req);
    if (result.has_value()) { return std::move(result.value()); }
    return it->second(req);
}

} // namespace STNL
