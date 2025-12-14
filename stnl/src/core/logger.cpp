
#include "stnl/core/logger.hpp"

#include <boost/asio.hpp>

#include <chrono>  // For std::chrono::system_clock
#include <ctime>   // For std::time_t and std::localtime
#include <iomanip> // For std::put_time and std::setw
#include <iostream>
#include <sstream>
#include <thread>
#include <utility>

namespace asio = boost::asio;

namespace STNL {
std::unique_ptr<asio::strand<asio::io_context::executor_type>> Logger::strand_ = nullptr;

LogStream::LogStream(std::string logType) : logType_(std::move(logType)) {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration) % std::chrono::seconds(1);
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm *tm = std::localtime(&now_c);
    std::ostringstream oss;
    // 1. Format Time (YYYY-MM-DD HH:MM:SS.mmm)
    // The format "%Y-%m-%d %H:%M:%S" is standard; append milliseconds.
    oss << "##[" << std::put_time(tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << static_cast<int>(ms.count()) << "][" << logType_
        << "]\n";
    buffer_ << oss.str();
}

// The core logic runs when the LogStream object is destroyed (goes out of
// scope).
LogStream::~LogStream() {
    Logger::Log(buffer_.str());
}

void Logger::Init(asio::io_context &ioc) {
    strand_ = std::make_unique<asio::strand<asio::io_context::executor_type>>(ioc.get_executor());
}

void Logger::Log(std::string msg) {
    if (!strand_) {
        std::cout << msg << '\n';
        return;
    }
    // Post the actual logging to the strand to ensure thread-safe, sequential
    // writes. We capture the strings by value (using std::move on the temporary
    // arguments) to ensure they are copied safely into the heap-allocated
    // lambda closure.
    asio::post(*strand_, [msg = std::move(msg)]() { std::cout << msg << '\n'; });
}

auto Logger::Dbg() -> LogStream {
    return {"DBG"};
}
auto Logger::Inf() -> LogStream {
    return {"INF"};
}
auto Logger::Err() -> LogStream {
    return {"ERR"};
}
auto Logger::Wrn() -> LogStream {
    return {"WRN"};
}

void Logger::Dbg(std::string_view msg) {
    Dbg() << std::string(msg);
}
void Logger::Inf(std::string_view msg) {
    Inf() << std::string(msg);
}
void Logger::Err(std::string_view msg) {
    Err() << std::string(msg);
}
void Logger::Wrn(std::string_view msg) {
    Wrn() << std::string(msg);
}
} // namespace STNL
