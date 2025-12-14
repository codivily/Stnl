#include "stnl/core/stnl_module.hpp"
#include "stnl/http/server.hpp"
#include <memory>

namespace STNL {

STNLModule::STNLModule(Server &server) : server_(server) {}
} // namespace STNL
