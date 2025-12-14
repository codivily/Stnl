#ifndef STNL_CORE_HPP
#define STNL_CORE_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <functional>
#include <string>
#include <unordered_map>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

namespace STNL {
class Request; // forward declaration

using HttpRequestBody = http::string_body;
using HttpRequest = http::request<HttpRequestBody>;

// HttpHandler type: processes the request and populate the response.
using RouteHandler = std::function<http::message_generator(const Request &)>;

// Route key: HTTP method + path
using Route = std::pair<http::verb, std::string>;
struct RouteHash {
    std::size_t operator()(const Route &key) const noexcept {
        std::size_t h1 = static_cast<std::size_t>(static_cast<unsigned short>(key.first));
        std::size_t h2 = std::hash<std::string>{}(key.second);
        return (h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2)));
    }
};
using Router = std::unordered_map<Route, RouteHandler, RouteHash>;
} // namespace STNL

#endif // STNL_TYPES_HPP
