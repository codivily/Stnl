#include "stnl/http/session.hpp"
#include "stnl/core/config.hpp"
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
    // Reset parser for new request
    parser_.emplace();
    
    // Get body limit from config or use default
    size_t bodyLimit = DEFAULT_BODY_LIMIT;
    auto configLimit = Config::Value<int64_t>("http.maxBodySize");
    if (configLimit.has_value() && configLimit.value() > 0) {
        bodyLimit = static_cast<size_t>(configLimit.value());
    }
    parser_->body_limit(bodyLimit);
    
    // Set request timeout
    stream_.expires_after(REQUEST_TIMEOUT);
    
    http::async_read(stream_, buffer_, *parser_, beast::bind_front_handler(&Session::OnRead, shared_from_this()));
}

void Session::OnRead(beast::error_code ec, std::size_t /*bytes_transferred*/) {
    // Cancel timeout
    beast::get_lowest_layer(stream_).expires_never();
    
    if (ec == http::error::end_of_stream) {
        beast::error_code ec2;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec2);
        return;
    }
    
    if (ec == http::error::body_limit) {
        Logger::Wrn() << "Session::OnRead: Request body too large";
        // Send 413 Payload Too Large
        auto makeResponse = [&]() -> http::message_generator {
            http::response<http::string_body> res{http::status::payload_too_large, 11};
            res.set(http::field::server, "STNL");
            res.set(http::field::content_type, "text/plain");
            res.body() = "Request body too large";
            res.prepare_payload();
            return res;
        };
        beast::async_write(stream_, makeResponse(), beast::bind_front_handler(&Session::OnWrite, shared_from_this()));
        return;
    }
    
    if (ec) {
        Logger::Err() << "Session::OnRead: " << ec.message();
        return;
    }
    
    Request req = Request::parse(parser_->get());
    http::message_generator res = HandleRequest(req);
    keepAlive_ = res.keep_alive();
    beast::async_write(stream_, std::move(res), beast::bind_front_handler(&Session::OnWrite, shared_from_this()));
}

void Session::OnWrite(beast::error_code ec, std::size_t /*bytes_transferred*/) {
    if (ec) {
        Logger::Err() << "Session::OnWrite: " << ec.message();
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
    static const std::unordered_map<std::string, std::string> mimeTypes = {
        // Text
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".txt", "text/plain"},
        {".csv", "text/csv"},
        {".xml", "text/xml"},
        
        // JavaScript
        {".js", "application/javascript"},
        {".mjs", "application/javascript"},
        {".json", "application/json"},
        
        // Images
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".ico", "image/x-icon"},
        {".webp", "image/webp"},
        {".bmp", "image/bmp"},
        
        // Fonts
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".ttf", "font/ttf"},
        {".otf", "font/otf"},
        {".eot", "application/vnd.ms-fontobject"},
        
        // Audio/Video
        {".mp3", "audio/mpeg"},
        {".wav", "audio/wav"},
        {".mp4", "video/mp4"},
        {".webm", "video/webm"},
        
        // Archives
        {".zip", "application/zip"},
        {".gz", "application/gzip"},
        {".tar", "application/x-tar"},
        
        // Documents
        {".pdf", "application/pdf"},
        {".doc", "application/msword"},
        {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    };
    
    std::string ext = filePath.extension().string();
    auto it = mimeTypes.find(ext);
    if (it != mimeTypes.end()) {
        return it->second;
    }
    
    Logger::Dbg() << "GetFileMimeType: Unknown extension: " << ext;
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
    // Run middleware chain FIRST (before route matching)
    boost::optional<http::message_generator> middlewareResult = ApplyMiddlewares(req);
    if (middlewareResult.has_value()) { 
        return std::move(middlewareResult.value()); 
    }
    
    // Extract path-only (strip query) for routing
    std::string_view full_target{parser_->get().target()};
    std::string_view path = full_target;
    size_t query_pos = full_target.find('?');
    if (query_pos != std::string_view::npos) { path = full_target.substr(0, query_pos); }
    
    Route key{parser_->get().method(), std::string(path)};
    auto it = server_.GetRouter().find(key); 
    
    if (it == server_.GetRouter().end()) {
        // Handle API routes
        if (path.starts_with("/api/") || path == "/api") {
            return Server::Response(req, http::status::not_found);
        }
        
        // Handle static files
        if (parser_->get().method() == http::verb::get) {
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
        }
        return Server::Response(req, http::status::not_found);
    }
    
    // Execute matched route handler
    return it->second(req);
}

} // namespace STNL
