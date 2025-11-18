#ifndef MAIN_HTTP_HANDLER_HPP
#define MAIN_HTTP_HANDLER_HPP

#include "stnl/core.hpp"
#include "stnl/server.hpp"
#include "stnl/stnl_module.hpp"

#include <boost/asio.hpp>

namespace asio = boost::asio;

class ServerMain: public STNL::STNLModule {

  public:
    static const char sType{};
    ServerMain(std::shared_ptr<STNL::Server> server);
    void Setup() override;
    void Launch() override;
    http::message_generator webGetHome(const STNL::HttpRequest& req);
};

#endif // MAIN_HTTP_HANDLER_HPP