
#include "stnl/blueprint.hpp"
#include "stnl/column.hpp"
#include "stnl/utils.hpp"

#include <boost/optional.hpp>

#include <map>
#include <vector>
#include <string>

namespace STNL {

  Blueprint::Blueprint(const std::string tableName) : tableName_(std::move(tableName)) {}
  
  const std::string& Blueprint::GetTableName() { return tableName_; }
  const std::map<std::string, Column>& Blueprint::GetColumns() { return columns_; }

  Column& Blueprint::GetOrAddColumn(std::string realName) {
    std::string name = Utils::StringToLower(realName);
    std::map<std::string, Column>::iterator it = columns_.find(name);
    if (it == columns_.end()) {
      auto result = columns_.emplace(name, Column{std::move(realName), ColumnType::Undefined});
      // result.second is true (inserted), result.first is the iterator
      return result.first->second;
    }
    return it->second;
  }

  BigIntProxy Blueprint::BigInt(std::string name) {
    Column& col = GetOrAddColumn(std::move(name));
    return BigIntProxy{col};
  }

  IntegerProxy Blueprint::Integer(std::string name) {
    Column& col = GetOrAddColumn(std::move(name));
    return IntegerProxy{col};
  }

  SmallIntProxy Blueprint::SmallInt(std::string name) {
    Column& col = GetOrAddColumn(std::move(name));
    return SmallIntProxy{col};
  }

  NumericProxy Blueprint::Numeric(std::string name) {
    Column& col = GetOrAddColumn(std::move(name));
    return NumericProxy{col};
  }

  BitProxy Blueprint::Bit(std::string name) {
    Column& col = GetOrAddColumn(std::move(name));
    return BitProxy{col};
  }

  CharProxy Blueprint::Char(std::string name) {
    Column& col = GetOrAddColumn(std::move(name));
    return CharProxy{col};
  }

  VarcharProxy Blueprint::Varchar(std::string name) {
    Column& col = GetOrAddColumn(std::move(name));
    return VarcharProxy{col};
  }

  BooleanProxy Blueprint::Boolean(std::string name) {
    Column& col = GetOrAddColumn(std::move(name));
    return BooleanProxy{col};
  }

  DateProxy Blueprint::Date(std::string name) {
    Column& col = GetOrAddColumn(std::move(name));
    return DateProxy{col};
  }

  TimestampProxy Blueprint::Timestamp(std::string name) {
    Column& col = GetOrAddColumn(std::move(name));
    return TimestampProxy{col};
  }

  UUIDProxy Blueprint::UUID(std::string name) {
    Column& col = GetOrAddColumn(std::move(name));
    return UUIDProxy{col};
  }

  TextProxy Blueprint::Text(std::string name) {
    Column& col = GetOrAddColumn(std::move(name));
    return TextProxy(col);
  }
}