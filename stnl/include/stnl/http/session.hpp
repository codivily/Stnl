#ifndef STNL_SESSION_HPP
#define STNL_SESSION_HPP

#include "stnl/http/core.hpp"

#include <boost/beast/core.hpp>
#include <iostream> // For error logging
#include <memory>

namespace beast = boost::beast;

namespace STNL {
class Request; // forward declaration
class Server;  // forward declaration

class Session : public std::enable_shared_from_this<Session> {
  public:
    explicit Session(tcp::socket socket, Server &server);
    void Run();

  private:
    void DoRead();
    void OnRead(beast::error_code ec, std::size_t bytes_transferred);
    void OnWrite(beast::error_code ec, std::size_t bytes_transferred);
    http::message_generator HandleRequest(Request &req);
    boost::optional<http::message_generator> ApplyMiddlewares(Request &req);
    void SetupTimeout();
    void OnTimeout(beast::error_code ec);

    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    boost::optional<http::request_parser<http::string_body>> parser_;  // Use optional parser for proper reset
    Server &server_;
    bool keepAlive_;
    
    static constexpr size_t DEFAULT_BODY_LIMIT = 10 * 1024 * 1024;  // 10MB
    static constexpr std::chrono::seconds REQUEST_TIMEOUT{30};       // 30 seconds
};
} // namespace STNL

#endif // STNL_SESSION_HPP
