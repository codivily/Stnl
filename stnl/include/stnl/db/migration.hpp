#ifndef STNL_DB_MIGRATION_HPP
#define STNL_DB_MIGRATION_HPP

#include "stnl/db/blueprint.hpp"
#include "stnl/db/sr_blueprint.hpp"

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace STNL {
class Migration {
  public:
    Migration() = default;
    void Table(std::string const &tableName, const std::function<void(Blueprint &)> &adaptFn);
    std::vector<std::string> const &GetTableNames() const;
    std::unordered_map<std::string, Blueprint> const &GetBlueprints() const;

    void Procedure(std::string const &spName, const std::function<void(SrBlueprint &)> &adaptFn);
    std::vector<std::string> const &GetProcedureNames() const;
    std::unordered_map<std::string, SrBlueprint> const &GetProcedureBlueprints() const;

  private:
    std::vector<std::string> tableNames_;
    std::unordered_map<std::string, Blueprint> blueprints_;

    std::vector<std::string> spNames_;
    std::unordered_map<std::string, SrBlueprint> spBlueprints_;
};
} // namespace STNL

#endif // STNL_DB_MIGRATION_HPP
