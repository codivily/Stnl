#ifndef STNL_SESSION_HPP
#define STNL_SESSION_HPP

#include "core.hpp"

#include <boost/beast/core.hpp>
#include <memory>
#include <iostream>  // For error logging


namespace beast = boost::beast;

namespace STNL {
    class Request; // forward declaration
    class Server; // forward declaration

    class Session : public std::enable_shared_from_this<Session> {
    public:
        explicit Session(tcp::socket socket, std::shared_ptr<Server> server);
        void Run();
    
    private:
        void DoRead();
        void OnRead(beast::error_code ec, std::size_t bytes_transferred);
        void OnWrite(beast::error_code ec, std::size_t bytes_transferred);
        http::message_generator HandleRequest(Request& req);
        boost::optional<http::message_generator> ApplyMiddlewares(Request& req);
    
        beast::tcp_stream stream_;  // Fixed typo: was "stram_"
        beast::flat_buffer buffer_;
        HttpRequest httpReq_;
        std::shared_ptr<Server> server_;  // Access routes & middleware
        bool keepAlive_;
    };
}


#endif // STNL_SESSION_HPP