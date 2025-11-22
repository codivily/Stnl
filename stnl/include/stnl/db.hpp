#ifndef STNL_DB_UTILS_HPP
#define STNL_DB_UTILS_HPP

#include "connection_pool.hpp"
#include "column.hpp"
#include <boost/asio.hpp>
#include <pqxx/pqxx>

#include <thread>
#include <future>
#include <string>

namespace asio = boost::asio;

namespace STNL {
  //class ConnectionPool; // Forware declaration

  struct QResult
  {
    pqxx::result data;
    bool ok;
    std::string msg;
  };

  class DB {
    public:
      DB(std::string& connStr, asio::io_context& ioc, size_t poolSize = 4, size_t numThreads = 6);
      ~DB();
      QResult Exec(std::string_view qSQL, bool silent = true);
      std::future<QResult> ExecAsync(std::string_view qSQL, bool silent = true);
      
      bool TableExists(std::string_view tableName);

      static std::string GetTypeSQL(Column& column);

    private:
      ConnectionPool pool_;
      asio::io_context& ioc_;
      asio::executor_work_guard<asio::io_context::executor_type> workGuard_;
      std::vector<std::thread> threadPool_;
  };
}

#endif // STNL_DB_UTILS_HPP