
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
    
    // ANSI color codes
    const char* color;
    const char* reset = "\033[0m";
    
    if (logType_ == "DBG") {
        color = "\033[36m";  // Cyan
    } else if (logType_ == "INF") {
        color = "\033[32m";  // Green
    } else if (logType_ == "WRN") {
        color = "\033[33m";  // Yellow
    } else if (logType_ == "ERR") {
        color = "\033[31m";  // Red
    } else {
        color = "\033[37m";  // White
    }
    
    std::ostringstream oss;
    // Format: [HH:MM:SS.mmm] LEVEL  message
    oss << "[" 
        << std::put_time(tm, "%H:%M:%S") << '.' 
        << std::setfill('0') << std::setw(3) << static_cast<int>(ms.count()) 
        << "] "
        << color << std::setw(5) << std::left << logType_ << reset << " ";
    buffer_ << oss.str();
}

// The core logic runs when the LogStream object is destroyed (goes out of
// scope).
LogStream::~LogStream() {
    std::string message = buffer_.str();
    
    // Check if message contains newlines (multi-line content)
    size_t pos = 0;
    size_t found = message.find('\n', pos);
    
    if (found != std::string::npos && found < message.length() - 1) {
        // Multi-line message detected - indent continuation lines
        // The indent should align with where the message starts (after timestamp and level)
        const std::string indent = "                    "; // 20 spaces to align after "[HH:MM:SS.mmm] LEVEL "
        
        std::string formatted;
        formatted.reserve(message.size() + 100); // Reserve extra space for indents
        
        while (found != std::string::npos) {
            // Append everything up to and including the newline
            formatted.append(message, pos, found - pos + 1);
            pos = found + 1;
            
            // If there's more content after this newline, add indent
            if (pos < message.length()) {
                formatted.append(indent);
            }
            
            found = message.find('\n', pos);
        }
        
        // Append any remaining content
        if (pos < message.length()) {
            formatted.append(message, pos, message.length() - pos);
        }
        
        Logger::Log(formatted);
    } else {
        // Single-line message - log as-is
        Logger::Log(message);
    }
}

void Logger::Init(asio::io_context &ioc) {
    strand_ = std::make_unique<asio::strand<asio::io_context::executor_type>>(ioc.get_executor());
}

void Logger::Log(std::string msg) {
    if (!strand_) {
        std::cout << msg << std::endl;  // Use endl for immediate flush
        return;
    }
    // Post the actual logging to the strand to ensure thread-safe, sequential
    // writes. We capture the strings by value (using std::move on the temporary
    // arguments) to ensure they are copied safely into the heap-allocated
    // lambda closure.
    asio::post(*strand_, [msg = std::move(msg)]() { std::cout << msg << std::endl; });
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
