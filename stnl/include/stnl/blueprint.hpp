#ifndef STNL_BLUEPRINT_HTPP
#define STNL_BLUEPRINT_HTPP


#include "column.hpp"

#include <boost/optional.hpp>

#include <map>
#include <vector>
#include <string>

namespace STNL {
  class BigIntProxy; // Forward declaration
  class IntegerProxy; // Forward declaration
  class SmallIntProxy; // Forward declaration
  class NumericProxy; // Forward declaration
  class BitProxy; // Forward declaration
  class CharProxy; // Forward declaration
  class VarcharProxy; // Forward declaration
  class BooleanProxy; // Forward declaration
  class DateProxy; // Forward declaration
  class TimestampProxy; // Forward declration
  class UUIDProxy; // Forward declaration
  class TextProxy; // Forward declaration

  class Blueprint {
    public:
      explicit Blueprint(const std::string tableName);
      virtual ~Blueprint() = default;
      std::string& GetTableName();
      const std::map<std::string, Column>& GetColumns();

      BigIntProxy BigInt(std::string name);
      IntegerProxy Integer(std::string name);
      SmallIntProxy SmallInt(std::string name);
      NumericProxy Numeric(std::string name);
      BitProxy Bit(std::string name);
      CharProxy Char(std::string name);
      VarcharProxy Varchar(std::string name);
      BooleanProxy Boolean(std::string name);
      DateProxy Date(std::string name);
      TimestampProxy Timestamp(std::string name);
      UUIDProxy UUID(std::string name);
      TextProxy Text(std::string name);

      void AddColumn(Column&& col);
    private:
      std::string tableName_;
      std::map<std::string, Column> columns_;

      Column& GetOrAddColumn(std::string realName);

  };
}

#endif //STNL_BLUEPRINT_HTPP