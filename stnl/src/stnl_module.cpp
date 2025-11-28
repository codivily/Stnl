#include "stnl/stnl_module.hpp"
#include "stnl/server.hpp"
#include <memory>


namespace STNL {

STNLModule::STNLModule(Server& server) : server_(server) {}
}  // namespace STNL