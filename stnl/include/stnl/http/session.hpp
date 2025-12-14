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

    beast::tcp_stream stream_; // Fixed typo: was "stram_"
    beast::flat_buffer buffer_;
    HttpRequest httpReq_;
    Server &server_; // Access routes & middleware
    bool keepAlive_;
};
} // namespace STNL

#endif // STNL_SESSION_HPP
