#ifndef STNL_LOGGER_HPP
#define STNL_LOGGER_HPP

#include <memory>
#include <boost/asio.hpp>

namespace asio = boost::asio;

namespace STNL
{
  class Logger
  {
  public:
  static void Init(asio::io_context& ioc);
  static void Log(std::string logType, std::string msg);
  static void Dbg(std::string msg);
  static void Inf(std::string msg);
  static void Err(std::string msg);
  static void Wrn(std::string msg);

  private:
    Logger() = delete;
    static std::unique_ptr<asio::strand<asio::io_context::executor_type>> strand_;
  };

} // namespace STNL

#endif // STNL_LOGGER_HPP