

#include "modules/ticker/ticker.hpp"
#include "stnl/core/logger.hpp"
#include "stnl/core/stnl_module.hpp"
#include "stnl/http/server.hpp"

using Logger = STNL::Logger;
using STNLModule = STNL::STNLModule;

Ticker::Ticker(Server &server) : STNLModule(server), counter_(0) {}

void Ticker::Reset() {
    counter_ = 0;
}

auto Ticker::Increment() -> int {
    ++counter_;
    return counter_;
}

auto Ticker::Decrement() -> int {
    if (counter_ > 0) { --counter_; }
    return counter_;
}

auto Ticker::GetValue() const -> int {
    return counter_;
}

void Ticker::Setup() {
    Logger::Dbg() << ("Ticker::Setup()");
}

void Ticker::Launch() {
    Logger::Dbg() << ("Ticker::Launch()");
}
