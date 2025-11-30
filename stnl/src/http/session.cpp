#include "stnl/http/server.hpp"
#include "stnl/http/core.hpp"
#include "stnl/http/request.hpp"
#include "stnl/http/session.hpp"
#include "stnl/core/logger.hpp"
#include "stnl/http/middleware.hpp"

#include <boost/beast/http/read.hpp>  // For async_read
#include <boost/beast/http/write.hpp>  // For async_write
#include <boost/beast/version.hpp>     // For BOOST_BEAST_VERSION_STRING


#include <string>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <algorithm>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

namespace STNL {

Session::Session(tcp::socket socket, Server& server)
    : stream_(std::move(socket)), server_(server) {}

void Session::Run() { DoRead(); }

void Session::DoRead() {
    httpReq_ = HttpRequest{};
    http::async_read(stream_, buffer_, httpReq_,
        beast::bind_front_handler(&Session::OnRead, shared_from_this()));
}

void Session::OnRead(beast::error_code ec, std::size_t) {  // bytes_transferred not used
    if (ec == http::error::end_of_stream) {
        beast::error_code ec2;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec2);  // Fixed: was "shutdown_end"
        return;
    }
    if (ec) {
        std::cerr << "Session::OnRead: ERROR: " << ec.message() << std::endl;
        return;
    }
    Request req = Request::parse(httpReq_);  // Wrap the HttpRequest for easier access
    http::message_generator res = HandleRequest(req);
    keepAlive_ = res.keep_alive();
    beast::async_write(stream_, std::move(res),
        beast::bind_front_handler(&Session::OnWrite, shared_from_this()));
}

void Session::OnWrite(beast::error_code ec, std::size_t) {  // bytes_transferred not used
    if (ec) {
        std::cerr << "Session::OnWrite: ERROR: " << ec.message() << std::endl;
        return;
    }
    if (!keepAlive_) {  // Fixed: was "need_eof()" which isn't standard; use keep_alive check
        beast::error_code ec2;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec2);
    } else {
        DoRead();
    }
}

boost::optional<http::message_generator> Session::ApplyMiddlewares(Request& req) {
    for (const std::unique_ptr<Middleware>& mw : server_.GetMiddlewares()) { 
        boost::optional<http::message_generator> result = mw->invoke(req);
        if (result.has_value()) {
            return std::move(result);
        }
    }
    return boost::none;
}

http::message_generator Session::HandleRequest(Request& req) {
    // FIXED: Extract path-only (strip query) for routing
    std::string_view full_target{httpReq_.target()};
    std::string_view path = full_target;
    size_t query_pos = full_target.find('?');
    if (query_pos != std::string_view::npos) {
        path = full_target.substr(0, query_pos);
    }
    Route key{httpReq_.method(), std::string(path)};  // Use path-only
    auto it = server_.GetRouter().find(key);
    if (it == server_.GetRouter().end())
    {
        return Server::Response(req, std::string("Not Found (HTTP 404)"), http::status::not_found);
    }
    // Run the middleware chain before handler
    boost::optional<http::message_generator> result = ApplyMiddlewares(req);
    if (result.has_value()) {
        return std::move(result.value());
    }
    return it->second(req);
}

}  // namespace STNL
