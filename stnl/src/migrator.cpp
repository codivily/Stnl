
#include "stnl/migrator.hpp"
#include "stnl/blueprint.hpp"
#include "stnl/utils.hpp"
#include "stnl/logger.hpp"
#include "stnl/db.hpp"

#include <functional>
#include <future>
#include <string>
#include <memory>
#include <pqxx/pqxx>

#include <iostream>

namespace STNL {

  void Migrator::Table(const std::string& tableName, std::function<void(Blueprint&)> adaptFn) {
    std::string key = Utils::StringToLower(tableName);
    auto [it, inserted] = blueprints_.emplace(key, tableName);
    adaptFn(it->second);
  }

  void Migrator::Migrate(DB& db) {
    for(auto it = blueprints_.begin(); it != blueprints_.end(); ++it) {
      try {
        ApplyBlueprint(db, it->second);
      } catch(std::exception& e) {
        Logger::Err("Migrator::Migrate: Error: " + std::string(e.what()));
      }
    }
  }

  void Migrator::ApplyBlueprint(DB& db, Blueprint& bp) {
    Logger::Dbg("Migrator::ApplyBlueprint: tableName = " + std::string(bp.GetTableName()));
    std::future<QResult> f1 = db.ExecAsync("SELECT NOW()");
    std::future<QResult> f2 = db.ExecAsync("SELECT idproduct, name FROM tbl_product ORDER BY name ASC");
    QResult r = f1.get();
    if (r.ok) {
      for(const auto& row : r.data) {
        Logger::Inf("Q:::NOW: " + std::string(row[0].c_str()));
      }
    }
    else { Logger::Err("Q:::NOW: ERROR" + r.msg); }
  
    r = f2.get();
    if (r.ok) {
      for(const auto& row : r.data) {
        Logger::Inf("Q:::PRODUCT: idproduct: " + std::string(row[0].c_str()) + ", name: " + std::string(row[1].c_str()));
      }
    }
    else { Logger::Err("Q:::PRODUCT: ERROR" + r.msg); }
  }
}