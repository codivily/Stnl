#ifndef STNL_UTILS_HPP
#define STNL_UTILS_HPP

#include <boost/asio.hpp>
#include <string>
#include <future>

namespace asio = boost::asio;

namespace STNL {
  class Utils {
    public:
      static std::string StringToLower(const std::string_view s);
      static std::string StringToUpper(const std::string_view s);
      static std::string TrimLeft(const std::string_view s);
      static std::string TrimRight(const std::string_view s);
      static std::string Trim(const std::string_view s);
      static std::string FixIndent(const std::string_view s);
      
      template <typename ResultType>
      static std::future<ResultType> AsFuture(asio::io_context& ioc, std::function<ResultType()> fn) {
          using TaskType = std::packaged_task<ResultType()>; 
          auto task = std::make_shared<TaskType>(std::move(fn)); 
          std::future<ResultType> fut = task->get_future();
          asio::post(ioc, [task] { (*task)(); });
          return fut;
      }

    private:
      Utils();
  };
}

#endif // STNL_UTILS_HPP