#ifndef MAIN_HTTP_HANDLER_HPP
#define MAIN_HTTP_HANDLER_HPP

#include "stnl/core.hpp"
#include "stnl/request.hpp"
#include "stnl/server.hpp"
#include "stnl/stnl_module.hpp"

#include <boost/asio.hpp>

namespace asio = boost::asio;

using STNLModule = STNL::STNLModule;
using Request = STNL::Request;
using Server = STNL::Server;

class ServerMain: public STNLModule, public std::enable_shared_from_this<ServerMain> {
  
  public:
    inline static volatile const char sType{};
    ServerMain(Server& server);
    void Setup() override;
    void Launch() override;
    //
    http::message_generator WebGetHome(Request const& req);
    http::message_generator ApiPostData(Request const& req);
    http::message_generator ApiGetProduct(Request const& req);
};

#endif // MAIN_HTTP_HANDLER_HPP