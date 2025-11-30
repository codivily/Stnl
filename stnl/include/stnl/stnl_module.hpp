#ifndef STNL_STNL_MODULE_HPP
#define STNL_STNL_MODULE_HPP

#include "core.hpp"

#include <memory>

namespace STNL {

  class Server; // forward declaration

  class STNLModule {
    public:
    inline static volatile const char sType{};
    ~STNLModule() = default;
    STNLModule(Server& server);
    virtual void SetupMigrations() {};
    virtual void Setup() {};
    virtual void Launch() {};
    protected:
      Server& server_;
  };
}

#endif // STNL_STNL_MODULE_HPP