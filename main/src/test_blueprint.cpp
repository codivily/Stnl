
#include "stnl/blueprint.hpp"
#include "stnl/migrator.hpp"
#include "stnl/db.hpp"
#include "stnl/logger.hpp"
#include "stnl/utils.hpp"


#include <boost/asio.hpp>
#include <boost/json.hpp>

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
  DB db(connStr, ioc);
  // STNL::ConnectionPool pool{connStr, 4};
  // auto pCon = std::make_shared<pqxx::connection>("dbname=stnl_db user=postgres password=!stnl1301 host=localhost port=5432 options=-csearch_path=stnl_sch,public");
  Migrator migrator;
  
  migrator.Table("Product", [](Blueprint& bp) {
    bp.BigInt("id").Identity();
    bp.UUID("uuid").Default();
    bp.Varchar("name").Length(255).NotNull();
    bp.Numeric("price").Precision(7).Scale(2).Null();
    bp.Text("description").Null();
    bp.Timestamp("utcdt").NotNull().Default();
    bp.Bit("active").N(1).NotNull().Default();
  });

  // QResult r = db.ExecAsync("SELECT * FROM product ORDER BY utcdt DESC").get();
  // Logger::Err() << r.data;
  db.Insert("Product",
    std::make_pair("name", "Blender"),
    std::make_pair("price", 10.5),
    std::make_pair("description", "Camouflage device - op.60%")
  );
  // migrator.Migrate(db);
  ioc.run();
  return 0;
}