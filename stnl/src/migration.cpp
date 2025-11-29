
#include "stnl/migration.hpp"
#include "stnl/blueprint.hpp"
#include "stnl/utils.hpp"


#include <functional>
#include <map>
#include <string>
#include <vector>


namespace STNL {
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

}