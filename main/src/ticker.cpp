

#include "ticker.hpp"
#include "stnl/stnl_module.hpp"
#include "stnl/server.hpp"
#include "stnl/logger.hpp"

using Logger = STNL::Logger;

Ticker::Ticker(std::shared_ptr<STNL::Server> server) : STNL::STNLModule(std::move(server)), counter_(0){}

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