

#include "database.hpp"
#include "stnl/stnl_module.hpp"
#include "stnl/server.hpp"
#include "stnl/logger.hpp"

#include <pqxx/pqxx>
#include <string>

using Logger = STNL::Logger;

Database::Database(std::shared_ptr<STNL::Server> server) : STNL::STNLModule(server){}


void Database::Setup() {
  Logger::Dbg() << ("Database::Setup()");
}

void Database::Launch() {
  Logger::Dbg() << ("Database::Launch()");
  try {
    pqxx::connection con("dbname=stnl_db user=postgres password=!stnl1301 host=localhost port=5432 options=-csearch_path=stnl_sch,public");
    Logger::Inf() << ("Connected to database successfully: " + std::string(con.dbname()));

    pqxx::work tx{con};
    pqxx::result r = tx.exec("SELECT NOW()");
    for (const auto& row : r) {
      Logger::Inf() << ("Current time from DB: " + std::string(row[0].c_str()));
    }
    for (auto [idproduct, name] : tx.stream<int, std::string_view>(("SELECT idproduct, name FROM tbl_product"))) {
      Logger::Inf() << ("Product ID: " + std::to_string(idproduct) + ", Name: " + std::string(name));
    }
    tx.commit();
    Logger::Inf() << ("Database::Launch():: Transaction committed successfully.");
  } catch(std::exception& e) {
    Logger::Err(std::string("Database connection error: ") + e.what());
  }
}