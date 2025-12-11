
#include "stnl/db/migration.hpp"
#include "stnl/db/sr_blueprint.hpp"
#include "stnl/db/blueprint.hpp"
#include "stnl/core/utils.hpp"


#include <functional>
#include <map>
#include <string>
#include <vector>


namespace STNL {

  // TABLE BLUEPRINT
  void Migration::Table(std::string const& tableName, std::function<void(Blueprint&)> adaptFn) {
    std::string key = Utils::StringToLower(tableName);
    auto [it, inserted] = blueprints_.emplace(key, tableName);
    if (inserted) { tableNames_.emplace_back(key); }
    adaptFn(it->second);
  }

  std::vector<std::string> const& Migration::GetTableNames() const {
    return tableNames_;
  }

  std::unordered_map<std::string, Blueprint> const& Migration::GetBlueprints() const {
    return blueprints_;
  }


  // STORED PROCEDURE BLUEPRINT
  void Migration::Procedure(std::string const& spName, std::function<void(SrBlueprint&)> adaptFn) {
    std::string key = Utils::StringToLower(spName);
    auto [it, inserted] = spBlueprints_.emplace(key, spName);
    if (inserted) { spNames_.emplace_back(key); }
    adaptFn(it->second);
  }
  std::vector<std::string> const& Migration::GetProcedureNames() const {
    return spNames_;
  }

  std::unordered_map<std::string, SrBlueprint> const& Migration::GetProcedureBlueprints() const {
    return spBlueprints_;
  }

}
