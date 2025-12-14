#ifndef STNL_DB_UTILS_HPP
#define STNL_DB_UTILS_HPP

#include "stnl/db/connection_pool.hpp"
#include "stnl/db/column.hpp"
#include "stnl/db/blueprint.hpp"
#include "stnl/db/migration.hpp"
#include "stnl/db/inserter.hpp"
#include "stnl/core/utils.hpp"
#include "stnl/core/logger.hpp"
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <pqxx/pqxx>

#include <functional>

#include <thread>
#include <future>
#include <string>
#include <memory>
#include <map>
#include <iostream>
#include <sstream>
#include <utility> // for std::forward
#include <type_traits> // Required for std::is_same_v

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
      DB(std::string const& connStr, asio::io_context& ioc, size_t poolSize = 4, size_t numThreads = 4);
      ~DB();

      asio::io_context& GetIOC();
      QResult Exec(std::string_view qSQL, bool silent = true);
      std::future<QResult> QExec(std::string_view qSQL, bool silent = true);

      QResult ExecSQLCmd(std::string const& sqlCmdName, std::string const& sqlCmd, pqxx::params& params, bool silent = true);
      
      template <typename ResultType>
      std::future<ResultType> QFuture(std::function<ResultType()> fn) {
          return Utils::AsFuture<ResultType>(ioc_, std::move(fn));
      }

      bool TableExists(std::string_view const tableName);
      std::vector<Column> GetTableColumns(std::string_view tableName = "");
      std::vector<std::string> GetTableIndexNames(std::string_view tableName);
      Blueprint QueryBlueprint(std::string_view tableName);
      std::unordered_map<size_t /*oid*/, std::string /*typname*/> const& GetDataTypes();
      static boost::json::value RowToJson(pqxx::row const& row, std::unordered_map<size_t, std::string> const& dataTypes);
      boost::json::value ConvertPQXXResultToJson(pqxx::result const& result);
      boost::json::value ConvertQResultToJson(QResult const& qResult);

      template<bool S = true, typename... Args>
      QResult Insert(std::string const& tableName, Args&& ...columnValuePairs) {
        Inserter inserter;
        (inserter << ... << std::forward<Args>(columnValuePairs));
        auto [SQLCmd, params] = inserter.flush(tableName);
        return ExecSQLCmd(std::format("sql_cmd_inert_{}", tableName), SQLCmd, params, S);
      }

      template<bool S = true, typename... Args>
      std::future<QResult> QInsert(std::string tableName, Args&& ...columnValuePairs) {
          return Utils::AsFuture<QResult>(ioc_, [this, tableName = std::move(tableName), columnValuePairs = std::make_tuple(std::forward<Args>(columnValuePairs)...)]() mutable {
              return std::apply([this, tableName = std::move(tableName)](auto&&... vals) {
                  return Insert<S>(tableName, std::forward<decltype(vals)>(vals)...);
              }, columnValuePairs);
          });
      }

      QResult InsertBatch(std::string const& tableName, std::function<void(BatchInserter& batch)> populateBatchFn);
      std::future<QResult> QInsertBatch(std::string const& tableName, std::function<void(BatchInserter& batch)> populateBatchFn);
      

      void Work(std::function<void(pqxx::work &tx)> doWorkFn);
      std::future<void> QWork(std::function<void(pqxx::work &tx)> doWork);
      
      Migration& GetMigration();
      
      static std::string GetConnectionString(
        std::string_view dbName,
        std::string_view dbUser,
        std::string_view dbPassword,
        std::string_view dbHost = "localhost",
        size_t dbPort = 5432,
        std::string_view dbSchema = "public"
      );

    private:
      ConnectionPool pool_;
      asio::io_context& ioc_;
      asio::executor_work_guard<asio::io_context::executor_type> workGuard_;
      std::vector<std::thread> threadPool_;
      std::unordered_map<size_t, std::string> dataTypes_;
      Migration migration_;
  };
}

#endif // STNL_DB_UTILS_HPP
