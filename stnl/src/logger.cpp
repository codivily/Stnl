
#include "stnl/logger.hpp"

#include <boost/asio.hpp>

#include <iostream>
#include <thread>
#include <sstream>
#include <chrono>  // For std::chrono::system_clock
#include <ctime>   // For std::time_t and std::localtime
#include <iomanip> // For std::put_time and std::setw

namespace asio = boost::asio;

namespace STNL
{
  std::unique_ptr<asio::strand<asio::io_context::executor_type>> Logger::strand_ = nullptr;

  LogStream::LogStream(const std::string &logType) : logType_(logType) {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration) % 1000;
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm *tm = std::localtime(&now_c);
    std::ostringstream oss;
    // 1. Format Time (YYYY-MM-DD HH:MM:SS.mmm)
    // The format "%Y-%m-%d %H:%M:%S" is standard.
    oss << "##[" << std::put_time(tm, "%Y-%m-%d %H:%M:%S") << "]["<< logType_ << "]\n";
    buffer_ << oss.str();
  }

  // The core logic runs when the LogStream object is destroyed (goes out of scope).
  LogStream::~LogStream() {
    Logger::Log(buffer_.str());
  }

  void Logger::Init(asio::io_context& ioc) {
    strand_ = std::make_unique<asio::strand<asio::io_context::executor_type>>(ioc.get_executor());
  }

  void Logger::Log(std::string msg) {
    if (!strand_) {
      std::cout << msg << std::endl;
      return;
    }
    // Post the actual logging to the strand to ensure thread-safe, sequential writes.
    // We capture the strings by value (using std::move on the temporary arguments)
    // to ensure they are copied safely into the heap-allocated lambda closure.
    asio::post(*strand_,
      [msg = std::move(msg)]() {
        std::cout << msg << std::endl;
    });
  }

  LogStream Logger::Dbg() { return LogStream("DBG"); }
  LogStream Logger::Inf() { return LogStream("INF"); }
  LogStream Logger::Err() { return LogStream("ERR"); }
  LogStream Logger::Wrn() { return LogStream("WRN"); }

  void Logger::Dbg(std::string_view msg) { Dbg() << std::string(msg); }
  void Logger::Inf(std::string_view msg) { Inf() << std::string(msg); }
  void Logger::Err(std::string_view msg) { Err() << std::string(msg); }
  void Logger::Wrn(std::string_view msg) { Wrn() << std::string(msg); }
} // namespace STNL
