
#include "stnl/migrator.hpp"
#include "stnl/blueprint.hpp"
#include "stnl/utils.hpp"
#include "stnl/logger.hpp"

#include <functional>
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

  void Migrator::Migrate(std::shared_ptr<pqxx::connection> pCon) {
    Logger::Err("Migrator::Migrate: Not Implemented");
  }
}