#ifndef STNL_CORE_HPP
#define STNL_CORE_HPP

#include <functional>
#include <string>
#include <unordered_map>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

namespace STNL
{
    using HttpRequestBody = http::string_body;
    using HttpRequest = http::request<HttpRequestBody>;
    
    // Middleware type: a function that takes request and response by reference.
    // It returns true to continue, false to stop (short-circuit).
    using Middleware = std::function<bool(HttpRequest&)>;
    
    // HttpHandler type: processes the request and populate the response.
    using RouteHandler = std::function<http::message_generator(const HttpRequest&)>;
    
    // Route key: HTTP method + path
    using Route = std::pair<http::verb, std::string>;
    struct RouteHash {
        std::size_t operator()(const Route& key) const noexcept {
            std::size_t h1 = static_cast<std::size_t>(static_cast<unsigned short>(key.first));
            std::size_t h2 = std::hash<std::string>{}(key.second);
            return (h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2)));
        }
    };
    using Router = std::unordered_map<Route, RouteHandler, RouteHash>;
} // namespace STNL

#endif // STNL_TYPES_HPP