#ifndef STNL_CORE_UTILS_HPP
#define STNL_CORE_UTILS_HPP

#include <boost/asio.hpp>
#include <future>
#include <string>
#include <vector>

namespace asio = boost::asio;

namespace STNL {
    class Utils {
    public:
    static auto StringToLower(const std::string_view s) -> std::string;
    static auto StringToUpper(const std::string_view s) -> std::string;
    static auto TrimLeft(const std::string_view s) -> std::string;
    static auto TrimRight(const std::string_view s) -> std::string;
    static auto Trim(const std::string_view s) -> std::string;
    static auto StringCaseCmp(const std::string_view a, std::string_view b) -> bool;
    static auto FixIndent(const std::string_view s) -> std::string;
    static auto Join(std::vector<std::string> const &parts, std::string const &separator = ";") -> std::string;
    template <typename ResultType>
    static auto AsFuture(asio::io_context &ioc, std::function<ResultType()> fn) -> std::future<ResultType> {
        using TaskType = std::packaged_task<ResultType()>;
        auto task = std::make_shared<TaskType>(std::move(fn));
        std::future<ResultType> fut = task->get_future();
        asio::post(ioc, [task] { (*task)(); });
        return fut;
    }
    };
} // namespace STNL

#endif // STNL_CORE_UTILS_HPP
