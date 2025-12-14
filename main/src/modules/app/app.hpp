#ifndef APP_HPP
#define APP_HPP

#include "stnl/core/stnl_module.hpp"
#include "stnl/http/core.hpp"
#include "stnl/http/request.hpp"
#include "stnl/http/server.hpp"

#include <boost/asio.hpp>

namespace asio = boost::asio;

using STNLModule = STNL::STNLModule;
using Request = STNL::Request;
using Server = STNL::Server;

class App : public STNLModule, public std::enable_shared_from_this<App> {

  public:
    inline static volatile const char sType{};
    App(Server &server);
    void SetupMigrations() override;
    void Setup() override;
    void Launch() override;
    //
    http::message_generator WebGetHome(Request const &req);
    http::message_generator ApiPostData(Request const &req);
    http::message_generator ApiGetProduct(Request const &req);
};

#endif // APP_HPP
