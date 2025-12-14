#ifndef STNL_UTILS_HPP
#define STNL_UTILS_HPP

#include <boost/asio.hpp>
#include <future>
#include <string>
#include <vector>

namespace asio = boost::asio;

namespace STNL {
namespace Utils {
std::string StringToLower(const std::string_view s);
std::string StringToUpper(const std::string_view s);
std::string TrimLeft(const std::string_view s);
std::string TrimRight(const std::string_view s);
std::string Trim(const std::string_view s);
bool StringCaseCmp(const std::string_view a, std::string_view b);
std::string FixIndent(const std::string_view s);
std::string Join(std::vector<std::string> const &parts, std::string const &separator = ";");

template <typename ResultType>
std::future<ResultType> AsFuture(asio::io_context &ioc, std::function<ResultType()> fn) {
    using TaskType = std::packaged_task<ResultType()>;
    auto task = std::make_shared<TaskType>(std::move(fn));
    std::future<ResultType> fut = task->get_future();
    asio::post(ioc, [task] { (*task)(); });
    return fut;
}
} // namespace Utils
} // namespace STNL

#endif // STNL_UTILS_HPP
