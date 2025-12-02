#ifndef STNL_MIGRATION_HPP
#define STNL_MIGRATION_HPP

#include "stnl/db/blueprint.hpp"
#include "stnl/db/sp_blueprint.hpp"

#include <functional>
#include <map>
#include <string>
#include <vector>


namespace STNL {
  class Migration {
    public:
      Migration() = default;
      void Table(std::string const& tableName, std::function<void(Blueprint&)> adaptFn);
      std::vector<std::string> const& GetTableNames() const;
      std::unordered_map<std::string, Blueprint> const& GetBlueprints() const;

      void Procedure(std::string const& spName, std::function<void(SpBlueprint&)> adaptFn);
      std::vector<std::string> const& GetProcedureNames() const;
      std::unordered_map<std::string, SpBlueprint> const& GetProcedureBlueprints() const;
    private:
      std::vector<std::string> tableNames_;
      std::unordered_map<std::string, Blueprint> blueprints_;

      std::vector<std::string> spNames_;
      std::unordered_map<std::string, SpBlueprint> spBlueprints_;
  };
}

#endif // STNL_MIGRATION_HPP
