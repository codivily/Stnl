#ifndef TICKER_HPP
#define TICKER_HPP

#include "stnl/http/server.hpp"
#include "stnl/core/stnl_module.hpp"

using Server = STNL::Server;
using STNLModule = STNL::STNLModule;

class Ticker : public STNLModule
{
public:
  inline static volatile const char sType{};
  Ticker(Server& server);
  void Setup() override;
  void Launch() override;
  void Reset();
  int Increment();
  int Decrement();
  int GetValue();

private:
  int counter_;
};

#endif // TICKER_HPP
