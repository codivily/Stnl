#include "stnl/logger.hpp"

#include <boost/asio.hpp>
#include <iostream>

namespace asio = boost::asio;

namespace STNL
{

  std::unique_ptr<asio::strand<asio::io_context::executor_type>> Logger::strand_;

  void Logger::Init(asio::io_context& ioc) {
    strand_ = std::make_unique<asio::strand<asio::io_context::executor_type>>(ioc.get_executor());
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
    if (!strand_) {
      std::cout << "[" + logType + "]: " << msg << std::endl;
      return;
    }
    asio::post(*strand_, [msg = std::move(msg), logType = std::move(logType)]() {
      std::cout << "[" + logType + "]: " << msg << std::endl;
    });
  }
} // namespace STNL
