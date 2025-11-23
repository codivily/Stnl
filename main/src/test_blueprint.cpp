
#include "stnl/blueprint.hpp"
#include "stnl/migrator.hpp"
#include "stnl/db.hpp"
#include "stnl/logger.hpp"
#include "stnl/utils.hpp"


#include <boost/asio.hpp>

#include <thread>
#include <memory>
#include <iostream>

namespace asio = boost::asio;

using Logger = STNL::Logger;

int main(int argc, char argv[]) {
  std::string connStr = "dbname=stnl_db user=postgres password=!stnl1301 host=localhost port=5432 options=-csearch_path=public";

  asio::io_context ioc;
  Logger::Init(ioc);
  STNL::DB db(connStr, ioc);
  // STNL::ConnectionPool pool{connStr, 4};
  // auto pCon = std::make_shared<pqxx::connection>("dbname=stnl_db user=postgres password=!stnl1301 host=localhost port=5432 options=-csearch_path=stnl_sch,public");
  STNL::Migrator migrator;
  
  migrator.Table("Product", [](STNL::Blueprint& bp) {
    bp.BigInt("idproduct");
    bp.UUID("uuid").Default("uuidv7()");
    bp.Varchar("name").Length(255).NotNull();
    bp.Numeric("price").Precision(7).Scale(2).Null();
    bp.Text("description").Null();
    bp.Timestamp("utcdt");
    bp.Bit("active").N(1).NotNull().Default("'1'::bit(1)");
  });

  using QResult = STNL::QResult;
  using Blueprint = STNL::Blueprint;

  Logger::Dbg() << "::main:: fut1";
  std::future<QResult> fut1 = db.AsFuture<QResult>([&db]() {
    Logger::Dbg() << "::main:: fut1 - Thread: " << std::this_thread::get_id();
    return db.Exec("SELECT NOW()");
  });
  Logger::Dbg() << "::main:: fut2";
  std::future<Blueprint> fut2 = db.AsFuture<Blueprint>([&db]() {
    Logger::Dbg() << "::main:: fut2 - Thread: " << std::this_thread::get_id();
    return db.QueryBlueprint("product");
  });
  Logger::Dbg() << "::main:: fut1.get()";
  QResult r = fut1.get();
  if (r.ok) {
    for(const auto& row: r.data) {
      Logger::Inf() << "::main:: DB TIME: " << std::string(row[0].c_str());
    }
  }
  Logger::Dbg() << "::main:: fut2.get()";
  Blueprint bp = fut2.get();
  Logger::Inf() << "::main:: Table: " << bp.GetTableName() << ", Columns.size() = " << bp.GetColumns().size();


  // migrator.Migrate(db);
  ioc.run(); // thre

  return 0;
}