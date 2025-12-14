
#include "stnl/blueprint.hpp"
#include "stnl/migrator.hpp"
#include "stnl/db.hpp"
#include "stnl/logger.hpp"
#include "stnl/utils.hpp"


#include <boost/asio.hpp>
#include <boost/json.hpp>

#include <chrono>
#include <thread>
#include <memory>
#include <iostream>
#include <format>
#include <string>
#include <utility> // for std::make_pair()

namespace asio = boost::asio;
namespace json = boost::json;

using Logger = STNL::Logger;
using DB = STNL::DB;
using Migrator = STNL::Migrator;
using Blueprint = STNL::Blueprint;
using QResult = STNL::QResult;
using BatchInserter = STNL::BatchInserter;

int main(int argc, char argv[]) {
  std::string dbName = "stnl_db";
  std::string dbUser = "postgres";
  std::string dbPassword = "postgres";
  std::string dbHost = "localhost";
  size_t dbPort = 5432;
  std::string dbSchema = "public";
  std::string connStr = DB::GetConnectionString(dbName, dbUser, dbPassword, dbHost, dbPort, dbSchema);
  
  asio::io_context ioc;
  Logger::Init(ioc);
  DB db(connStr, ioc, 24, 24);
  //
  Migrator migrator;
  migrator.Table("product", [](Blueprint& bp) {
    bp.BigInt("id").Identity().Unique();
    bp.UUID("uuid").Default().Index();
    bp.Varchar("name").Length(255).NotNull();
    bp.Numeric("price").Precision(9).Scale(2).Null();
    bp.Text("description").Null();
    bp.Timestamp("utcdt").NotNull().Index();
    bp.Bit("active").N(1).NotNull().Default();
  });

  migrator.Table("category", [](Blueprint& bp) {
    bp.BigInt("id").Identity().Unique();
    bp.UUID("uuid").Null().Index();
    bp.Varchar("name").NotNull().Default("'<empty>'");
    bp.Timestamp("utcdt").NotNull().Default();
    bp.Bit("active").N(1).NotNull().Default();
  });

  std::unordered_map<size_t, std::string> const& dataTypes = db.GetDataTypes();
  for(const auto& [oid, typname] : dataTypes) {
    Logger::Dbg() << "OID: " << oid << ", TYPE: " << typname;
  }

  QResult r = db.Exec("SELECT * FROM product ORDER BY uuid DESC LIMIT 10");
  if (r.ok) {
    Logger::Dbg() << db.ConvertQResultToJson(r) << '\n';
    Logger::Dbg() << db.ConvertPQXXResultToJson(r.data) << '\n';
  }


  migrator.Migrate(db);
  ioc.run();
}