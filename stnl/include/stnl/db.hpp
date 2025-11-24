#ifndef STNL_DB_UTILS_HPP
#define STNL_DB_UTILS_HPP

#include "connection_pool.hpp"
#include "blueprint.hpp"
#include "column.hpp"
#include "utils.hpp"
#include "logger.hpp"
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

namespace asio = boost::asio;

namespace STNL {

  struct QResult
  {
    pqxx::result data;
    bool ok;
    std::string msg;
    boost::json::value Json() const;
    boost::json::value DataJson() const;
    friend std::ostream& operator<<(std::ostream& os, QResult const& qResult);
    friend LogStream& operator<<(LogStream& stream, QResult const& qResult);
  };
  std::ostream& operator<<(std::ostream& os, QResult const& qResult);
  LogStream& operator<<(LogStream& stream, QResult const& qResult);


  enum PgFieldTypeOID {
    boolean = 16,
    character = 18,
    int8 = 20,
    int2 = 21,
    int4 = 23,
    text = 25,
    json = 114,
    xml = 142,
    float4 = 700,
    float8 = 701,
    money = 790,
    varchar = 1043,
    date = 1082,
    time = 1083,
    timestamp = 1114,
    timestamptz = 1184,
    timetz = 1266,
    bit = 1560,
    numeric = 1700,
    uuid = 2950,
    jsonb = 3802
  };

    
  class SelectedFieldIndex {
    public:
      SelectedFieldIndex() = default;
      std::string Field(std::string const& fieldName);
      unsigned int Index(std::string const& fieldName) const;
      std::string operator()(std::string const& fieldName);
    private:
      std::unordered_map<std::string, unsigned int> fieldToIndex_;
  };

  class DB {
    
    public:
      DB(std::string& connStr, asio::io_context& ioc, size_t poolSize = 4, size_t numThreads = 4);
      ~DB();

      asio::io_context& GetIOC();
      QResult Exec(std::string_view qSQL, bool silent = true);
      std::future<QResult> ExecAsync(std::string_view qSQL, bool silent = true);
      
      template <typename ResultType>
      std::future<ResultType> QFuture(std::function<ResultType()> fn) {
          return Utils::AsFuture<ResultType>(ioc_, std::move(fn));
      }

      bool TableExists(std::string_view tableName);
      std::vector<Column> GetTableColumns(std::string_view tableName = "");
      Blueprint QueryBlueprint(std::string_view tableName);
      
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
  };
}

#endif // STNL_DB_UTILS_HPP