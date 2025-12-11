
#include "stnl/db/blueprint.hpp"
#include "stnl/db/sr_blueprint.hpp"
#include "stnl/db/migration.hpp"
#include "stnl/db/migrator.hpp"
#include "stnl/db/db.hpp"
#include "stnl/core/logger.hpp"
#include "stnl/core/utils.hpp"


#include <boost/asio.hpp>
#include <boost/json.hpp>

#include <chrono>
#include <thread>
#include <memory>
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <utility> // for std::make_pair()

namespace asio = boost::asio;
namespace json = boost::json;

using Logger = STNL::Logger;
using DB = STNL::DB;
using Migration = STNL::Migration;
using Migrator = STNL::Migrator;
using Blueprint = STNL::Blueprint;
using SrBlueprint = STNL::SrBlueprint;
using QResult = STNL::QResult;
using BatchInserter = STNL::BatchInserter;

int main(int argc, char argv[]) {
  std::string dbName = "stnl_db";
  std::string dbUser = "postgres";
  std::string dbPassword = "!stnl1301";
  std::string dbHost = "localhost";
  size_t dbPort = 5432;
  std::string dbSchema = "public";
  std::string connStr = DB::GetConnectionString(dbName, dbUser, dbPassword, dbHost, dbPort, dbSchema);
  
  asio::io_context ioc;
  Logger::Init(ioc);
  DB db(connStr, ioc, 24, 24);
  //
  Migration migration;
  migration.Table("product", [](Blueprint& bp) {
    bp.BigInt("id").Identity().Unique();
    bp.UUID("uuid").Default().Index();
    bp.Varchar("name").Length(255).NotNull();
    bp.Numeric("price").Precision(9).Scale(2).Null();
    bp.Text("description").Null();
    bp.Timestamp("utcdt").NotNull().Index();
    bp.Bit("active").N(1).NotNull().Default();
  });

  migration.Table("category", [](Blueprint& bp) {
    bp.BigInt("id").Identity().Unique();
    bp.UUID("uuid").Null().Index();
    bp.Varchar("name").NotNull().Default("'<empty>'");
    bp.Timestamp("utcdt").NotNull().Default();
    bp.Bit("active").N(1).NotNull().Default();
  });
  
  migration.Procedure("sp_001", [](SrBlueprint &bp) {
      bp.BigInt("pv_id").NotNull();
      bp.BodyDelimiter() = "$$";
      bp.Body() = R"(
        DECLARE
          lv_name VARCHAR(255) DEFAULT NULL;
        BEGIN
          EXECUTE 'SELECT name FROM product WHERE id = $1' INTO lv_name USING pv_id;
          RAISE NOTICE 'Product Name: %', lv_name;
        END;
      )";
  });

  Migrator migrator;
  migrator.Migrate(db, migration);

  unsigned int numThreads = std::thread::hardware_concurrency();
  if (numThreads == 0) { numThreads = 1; }
  std::vector<std::thread> threads;
  threads.reserve(numThreads);
  for (unsigned int i = 0; i < numThreads; ++i) {
    threads.emplace_back([&ioc] { ioc.run(); });
  }
  /* wait for threads */
  for(std::thread& t : threads) { t.join(); }
}