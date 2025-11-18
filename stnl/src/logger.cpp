#include "stnl/logger.hpp"

#include <boost/asio.hpp>
#include <iostream>

namespace asio = boost::asio;

namespace STNL
{
  void Logger::Init(asio::io_context& ioc) {
    strand_ = asio::make_strand(ioc);
  }

  void Logger::Dbg(std::string msg) {
    Log("DBG", std::move(msg));
  }

  void Logger::Inf(std::string msg) {
    Log("INF", std::move(msg));
  }

  void Logger::Err(std::string msg) {
    Log("ERR", std::move(msg));
  }

  void Logger::Wrn(std::string msg) {
    Log("WRN", std::move(msg));
  }

  void Logger::Log(std::string logType, std::string msg) {
    asio::post(strand_, [msg, logType]() {
      std::cout << "[" + logType + "]: " << msg << std::endl;
    });
  }
} // namespace STNL
