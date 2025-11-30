#ifndef STNL_MIDDLEWARE_HPP
#define STNL_MIDDLEWARE_HPP

#include <boost/optional.hpp>
#include <boost/beast/http.hpp>

namespace http = boost::beast::http;

namespace STNL {

  class Request; // forward declaration
  class Server; // formward declaration
  
  class Middleware {
    public:
      virtual ~Middleware() = default;
      explicit Middleware(Server& server);
      virtual boost::optional<boost::beast::http::message_generator> invoke(Request& req) = 0;
      virtual void Setup() {};
    private:
      Server& server_;
  };

} // namespace STNL
#endif // STNL_MIDDLEWARE_HPP
