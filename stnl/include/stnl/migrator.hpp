
#ifndef STNL_MIGRATOR_HPP
#define STNL_MIGRATOR_HPP

#include "blueprint.hpp"

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
  };
}

#endif // STNL_MIGRATOR_HPP