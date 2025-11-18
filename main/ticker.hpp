#ifndef TICKER_HPP
#define TICKER_HPP

#include "stnl/server.hpp"
#include "stnl/stnl_module.hpp"

class Ticker : public STNL::STNLModule
{
public:
  inline static volatile const char sType{};
  Ticker(std::shared_ptr<STNL::Server> server);
  void Setup() override;
  void Launch() override;
  void Reset();
  void Increment();
  void Decrement();
  int GetValue();

private:
  int counter_;
};

#endif // TICKER_HPP