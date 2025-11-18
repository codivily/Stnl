#include "stnl/stnl_module.hpp"
#include "stnl/server.hpp"
#include <memory>


namespace STNL {

STNLModule::STNLModule(std::shared_ptr<Server> server) : server_(server) {}
std::shared_ptr<Server> STNLModule::GetServer() { return server_; }

}  // namespace STNL