#include "stnl/middleware.hpp"

namespace STNL {

  Middleware::Middleware(std::shared_ptr<Server> server) : server_(server) {}
  std::shared_ptr<Server> Middleware::GetServer() {
    return server_;
  }
  
}