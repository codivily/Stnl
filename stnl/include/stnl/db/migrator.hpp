
#ifndef STNL_MIGRATOR_HPP
#define STNL_MIGRATOR_HPP

#include "stnl/db/blueprint.hpp"
#include "stnl/db/column.hpp"
#include "stnl/db/migration.hpp"
#include "stnl/db/sr_blueprint.hpp"

#include <pqxx/pqxx>

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace STNL {
class DB;

class Migrator {
  public:
    Migrator() = default;
    static void Migrate(DB &db, Migration const &migration);
    static void Migrate(DB &db);

  private:
    static void ApplyBlueprint(DB &db, Blueprint const &bp);
    static void ApplyProcedureBlueprint(DB &db, SrBlueprint const &bp);
    static std::string GenerateSQLType(Column const &col);
    static std::string GenerateCreateSQL(Blueprint const &bp);
    static std::string GenerateSQLConstraints(Column const &col);

    // Stored procedure creation utilities
    static std::string GenerateSrParamSQL(SrParam const &spParam);
    // Internal helpers used by ApplyBlueprint. Kept private to avoid header
    // API exposure.
    static std::string BuildAddColumnSQL(std::string const &tableName, Column const &desiredCol);
    static std::string BuildAlterTypeSQL(std::string const &tableName, Column const &desiredCol);
    static std::string BuildIdentitySQL(std::string const &tableName, Column const &desiredCol);
    static std::string BuildNullabilitySQL(std::string const &tableName, Column const &desiredCol);
    static std::string BuildDefaultValueSQL(std::string const &tableName, Column const &desiredCol, std::string const &desiredDefaultValue);
    static std::string BuildUniqueIndexSQL(Column const &currentCol, Column const &desiredCol);
    static std::string BuildIndexSQL(Column const &currentCol, Column const &desiredCol);
    static void CollectAlterStatementsForColumn(std::string const &tableName, Column const &currentCol, Column const &desiredCol,
                                                std::vector<std::string> &alterStatements);
};
} // namespace STNL

#endif // STNL_MIGRATOR_HPP
