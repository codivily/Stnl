

#include "ticker.hpp"
#include "stnl/stnl_module.hpp"
#include "stnl/server.hpp"
#include <iostream>

Ticker::Ticker(std::shared_ptr<STNL::Server> server) : STNL::STNLModule(server), counter_(0){}

void Ticker::Reset() {
  counter_ = 0;
}

void Ticker::Increment() {
  ++counter_;
}

void Ticker::Decrement() {
  if (counter_ > 0) { --counter_; }
}

int Ticker::GetValue() {
  return counter_;
}

void Ticker::Setup() {
  std::cout << "Ticker::Setup()" << std::endl;
}

void Ticker::Launch() {
  std::cout << "Ticker::Launch()" << std::endl;
}