#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "stnl/server.hpp"
#include "stnl/stnl_module.hpp"

class Database : public STNL::STNLModule
{
public:
  inline static volatile const char sType{};
  Database(std::shared_ptr<STNL::Server> server);
  void Setup() override;
  void Launch() override;
};

#endif // DATABASE_HPP