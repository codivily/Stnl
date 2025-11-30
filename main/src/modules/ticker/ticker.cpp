

#include "modules/ticker/ticker.hpp"
#include "stnl/core/stnl_module.hpp"
#include "stnl/http/server.hpp"
#include "stnl/core/logger.hpp"

using Logger = STNL::Logger;
using STNLModule = STNL::STNLModule;

Ticker::Ticker(Server& server) : STNLModule(server), counter_(0){}

void Ticker::Reset() {
  counter_ = 0;
}

int Ticker::Increment() {
  ++counter_;
  return counter_;
}

int Ticker::Decrement() {
  if (counter_ > 0) { --counter_; }
  return counter_;
}

int Ticker::GetValue() {
  return counter_;
}

void Ticker::Setup() {
  Logger::Dbg() << ("Ticker::Setup()");
}

void Ticker::Launch() {
  Logger::Dbg() << ("Ticker::Launch()");
}
