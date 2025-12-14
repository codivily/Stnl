
#include "stnl/db/migration.hpp"
#include "stnl/core/utils.hpp"
#include "stnl/db/blueprint.hpp"
#include "stnl/db/sr_blueprint.hpp"

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace STNL {

// TABLE BLUEPRINT
void Migration::Table(std::string const &tableName, const std::function<void(Blueprint &)> &adaptFn) {
    std::string key = Utils::StringToLower(tableName);
    auto [it, inserted] = blueprints_.emplace(key, tableName);
    if (inserted) { tableNames_.emplace_back(key); }
    adaptFn(it->second);
}

auto Migration::GetTableNames() const -> std::vector<std::string> const & {
    return tableNames_;
}

auto Migration::GetBlueprints() const -> std::unordered_map<std::string, Blueprint> const & {
    return blueprints_;
}

// STORED PROCEDURE BLUEPRINT
void Migration::Procedure(std::string const &spName, const std::function<void(SrBlueprint &)> &adaptFn) {
    std::string key = Utils::StringToLower(spName);
    auto [it, inserted] = spBlueprints_.emplace(key, spName);
    if (inserted) { spNames_.emplace_back(key); }
    adaptFn(it->second);
}
auto Migration::GetProcedureNames() const -> std::vector<std::string> const & {
    return spNames_;
}

auto Migration::GetProcedureBlueprints() const -> std::unordered_map<std::string, SrBlueprint> const & {
    return spBlueprints_;
}

} // namespace STNL
