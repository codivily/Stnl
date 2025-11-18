#ifndef STNL_STNL_MODULE_HPP
#define STNL_STNL_MODULE_HPP

#include "core.hpp"

#include <memory>

namespace STNL {

  class Server; // forward declaration

  class STNLModule {
    public:
    static const char sType{};
    ~STNLModule() = default;
    STNLModule(std::shared_ptr<Server> server); // for accessing usefull things
    std::shared_ptr<Server> GetServer();
    virtual void Setup() {};
    virtual void Launch() {};
    private:
      std::shared_ptr<Server> server_;
  };
}

#endif // STNL_STNL_MODULE_HPP