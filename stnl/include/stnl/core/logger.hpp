#ifndef STNL_LOGGER_HPP
#define STNL_LOGGER_HPP

#include <memory>
#include <boost/asio.hpp>

namespace asio = boost::asio;

namespace STNL
{

  /**
   * @brief Temporary class designed to accumulate stream data (using <<) and execute
   * the final Logger::Log call upon destruction.
   */
  class LogStream
  {
  public:
    // Constructor takes the log type (e.g., "DBG", "INF") and a reference to the Logger.
    LogStream(const std::string &logType);

    // Copying the stream is usually undesirable, so we delete it.
    LogStream(const LogStream &) = delete;
    LogStream &operator=(const LogStream &) = delete;

    // The destructor is the key: it executes the final log write.
    ~LogStream();

    // The overloaded stream insertion operator. It's a template function
    // to accept any type (T) that supports operator<<.
    template <typename T>
    LogStream &operator<<(const T &msg)
    {
      // Write the message part into the internal stringstream
      buffer_ << msg;
      return *this;
    }

  private:
    std::stringstream buffer_;
    std::string logType_;
    // We need a way to reference the static Logger methods without being static itself.
    // In the final implementation, we'll make the Logger::Log static and accessible.
  };

  /**
   * @brief The main Logger class, adapted to return a temporary LogStream object.
   */
  class Logger
  {
  public:
    static void Init(asio::io_context &ioc);
    static void Log(std::string msg);
    // --- New Stream Accessor Methods ---
    // These methods return a temporary LogStream object.

    // Note: The methods must be defined to return something that can be streamed to.
    // We define a static function that returns a temporary LogStream object.
    static LogStream Dbg();
    static LogStream Inf();
    static LogStream Err();
    static LogStream Wrn();

    static void Dbg(std::string_view msg);
    static void Inf(std::string_view msg);
    static void Err(std::string_view msg);
    static void Wrn(std::string_view msg);

    // Private members remain
  private:
    Logger() = delete;
    static std::unique_ptr<asio::strand<asio::io_context::executor_type>> strand_;

    // Static instance of the Logger class (needed for LogStream constructor)
    static Logger instance_;
  };

} // namespace STNL

#endif // STNL_LOGGER_HPP