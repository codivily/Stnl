#ifndef STNL_DB_UTILS_HPP
#define STNL_DB_UTILS_HPP

#include "connection_pool.hpp"
#include "blueprint.hpp"
#include "column.hpp"
#include <boost/asio.hpp>
#include <pqxx/pqxx>

#include <thread>
#include <future>
#include <string>

namespace asio = boost::asio;

namespace STNL {

  struct QResult
  {
    pqxx::result data;
    bool ok;
    std::string msg;
  };

  class DB {
    public:
      DB(std::string& connStr, asio::io_context& ioc, size_t poolSize = 4, size_t numThreads = 4);
      ~DB();
      QResult Exec(std::string_view qSQL, bool silent = true);
      std::future<QResult> ExecAsync(std::string_view qSQL, bool silent = true);
      
      std::vector<Column> GetTableColumns(std::string_view tableName = "");
      Blueprint QueryBlueprint(std::string_view tableName);
      bool TableExists(std::string_view tableName);

      static std::string GetSQLType(Column& c);

    private:
      ConnectionPool pool_;
      asio::io_context& ioc_;
      asio::executor_work_guard<asio::io_context::executor_type> workGuard_;
      std::vector<std::thread> threadPool_;
  };
}

#endif // STNL_DB_UTILS_HPP