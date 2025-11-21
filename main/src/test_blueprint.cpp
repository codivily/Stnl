
#include "stnl/blueprint.hpp"
#include "stnl/migrator.hpp"
#include "stnl/utils.hpp"

#include <memory>
#include <iostream>

int main(int argc, char argv[]) {
  auto pCon = std::make_shared<pqxx::connection>("dbname=stnl_db user=postgres password=!stnl1301 host=localhost port=5432 options=-csearch_path=stnl_sch,public");
  STNL::Migrator migrator;
  
  migrator.Table("Product", [](STNL::Blueprint& bp) {
    bp.BigInt("idproduct").Identity();
    bp.Varchar("name").Length(255).NotNull();
    bp.Numeric("price").Precision(7).Scale(2).Null();
    bp.Text("description").Null().Default("1");
    bp.Timestamp("utcdt").NotNull();
    bp.Bit("active").N(1).NotNull().Default("1");
  });

  migrator.Migrate(pCon);

  return 0;
}