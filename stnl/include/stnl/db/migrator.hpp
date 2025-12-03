
#ifndef STNL_MIGRATOR_HPP
#define STNL_MIGRATOR_HPP

#include "stnl/db/column.hpp"
#include "stnl/db/sp_blueprint.hpp"
#include "stnl/db/blueprint.hpp"
#include "stnl/db/migration.hpp"

#include <pqxx/pqxx>

#include <functional>
#include <string>
#include <map>
#include <memory>

namespace STNL {
  class DB;

  class Migrator {
    public:
      Migrator() = default;
      void Migrate(DB& db, Migration const& migration);
      void Migrate(DB& db);
    private:
      void ApplyBlueprint(DB& db, Blueprint const& bp);
      void ApplyProcedureBlueprint(DB& db, SpBlueprint const& bp);
      std::string GenerateSQLType(Column const& col);
      std::string GenerateCreateSQL(Blueprint const& bp);
      std::string GenerateSQLConstraints(Column const& col);
  };
}

#endif // STNL_MIGRATOR_HPP
