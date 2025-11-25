
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
  std::string dbPassword = "!stnl1301";
  std::string dbHost = "localhost";
  size_t dbPort = 5432;
  std::string dbSchema = "public";
  std::string connStr = DB::GetConnectionString(dbName, dbUser, dbPassword, dbHost, dbPort, dbSchema);
  
  asio::io_context ioc;
  Logger::Init(ioc);
  DB db(connStr, ioc, 24, 24);
  // STNL::ConnectionPool pool{connStr, 4};
  // auto pCon = std::make_shared<pqxx::connection>("dbname=stnl_db user=postgres password=!stnl1301 host=localhost port=5432 options=-csearch_path=stnl_sch,public");
  // Migrator migrator;
  // migrator.Table("Product", [](Blueprint& bp) {
  //   bp.BigInt("id").Identity();
  //   bp.UUID("uuid").Default();
  //   bp.Varchar("name").Length(255).NotNull();
  //   bp.Numeric("price").Precision(9).Scale(2).Null();
  //   bp.Text("description").Null();
  //   bp.Timestamp("utcdt").NotNull().Default();
  //   bp.Bit("active").N(1).NotNull().Default();
  // });
  // migrator.Migrate(db);

  // QResult r = db.QExec("SELECT id, uuid, name, description, utcdt, active FROM product ORDER BY utcdt ASC LIMIT 15").get();
  // try {
  //   for(const pqxx::row& row : r.data) {
  //     auto [id, uuid, name, description, utcdt, active] = row.as<long long, char const*, char const*, char const*, char const*, bool>();
  //     Logger::Dbg() << std::format("{} - {} - {} - {} - {} - {}", id, uuid, name, (description == nullptr ? "<null>" : description), utcdt, active);
  //   }
  // } catch(std::exception& e) {
  //   Logger::Err() << "Error: " << std::string(e.what());
  // }

    
  Logger::Dbg() << "::main:: Before batch insert";
  std::future<QResult> fut = db.QInsertBatch("Product", [](BatchInserter& batch) {
    // add items to your batch insert
    const size_t batchSize = 100000;
    Logger::Dbg() << "::main:: batch:loop:start: bathcSize: " << std::to_string(batchSize);
    size_t i = 0;
    while (true) {
      batch
        << std::make_pair("name", "Green-Qeen - " + std::to_string(i))
        << std::make_pair("description", "Gk")
        << std::make_pair("price", 12.9)
        << std::make_pair("active", 1);
      batch.flush(); // do no forget to call the batch.flush()
      ++i;
      if (i >= batchSize) { break; }
    }
    Logger::Dbg() << "::main:: batch:loop:end";
  });
  Logger::Dbg() << "::main:: After batch inserter";
  Logger::Dbg() << "::main:: waiting batch to finish executing";
  fut.get();
  Logger::Dbg() << "::main:: batch finished executing";
  ioc.run();
}