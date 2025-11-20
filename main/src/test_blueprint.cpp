
#include "stnl/blueprint.hpp"
#include "stnl/column.hpp"
#include "stnl/utils.hpp"


int main(int argc, char argv[]) {

  STNL::Blueprint bp{"product"};

  bp.BigInt("idproduct").Identity();
  bp.Varchar("name").Length(255).Nullable(false);
  bp.Numeric("price").Precision(7).Scale(2).Nullable();
  bp.Timestamp("utcdt").Nullable();
  bp.Bit("active").Nullable(false).Default("1");

  return 0;
}