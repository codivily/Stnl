
#ifndef STNL_MIGRATOR_HPP
#define STNL_MIGRATOR_HPP

#include "blueprint.hpp"

#include <pqxx/pqxx>

#include <functional>
#include <string>
#include <map>
#include <memory>

namespace STNL {

  class Migrator {
    public:
      Migrator() = default;
      void Table(const std::string& tableName, std::function<void(Blueprint&)> adaptFn);
      void Migrate(std::shared_ptr<pqxx::connection> pCon);
    private:
      std::map<std::string, Blueprint> blueprints_;
  };
}

#endif // STNL_MIGRATOR_HPP