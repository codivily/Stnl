
#ifndef STNL_MIGRATOR_HPP
#define STNL_MIGRATOR_HPP

#include "blueprint.hpp"
#include "column.hpp"

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
      void Table(const std::string& tableName, std::function<void(Blueprint&)> adaptFn);
      void Migrate(DB& db);
    private:
      std::map<std::string, Blueprint> blueprints_;
      void ApplyBlueprint(DB& db, Blueprint& bp);
      std::string GenerateSQLType(const Column& col);
      std::string GenerateCreateSQL(Blueprint& bp);
      std::string GenerateSQLConstraints(const Column& col);
  };
}

#endif // STNL_MIGRATOR_HPP