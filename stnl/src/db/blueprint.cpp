
#include "stnl/db/blueprint.hpp"
#include "stnl/db/column.hpp"
#include "stnl/core/utils.hpp"

#include <boost/optional.hpp>

#include <map>
#include <vector>
#include <string>

namespace STNL {

  Blueprint::Blueprint(std::string const tableName) : tableName_(tableName) {}
  
  std::string const& Blueprint::GetTableName() const { return tableName_; }
  std::unordered_map<std::string, Column> const& Blueprint::GetColumns() const { return columns_; }

  Column& Blueprint::GetOrAddColumn(std::string realName) {
    std::string name = Utils::StringToLower(realName);
    auto it = columns_.find(name);
    if (it == columns_.end()) {
      auto [newIt, inserted] = columns_.emplace(name, Column{this->tableName_, std::move(realName), SQLDataType::Undefined});
      if (inserted) { columnNames_.emplace_back(name); }
      return newIt->second;
    }
    return it->second;
  }

  std::vector<std::string> const& Blueprint::GetColumnNames() const {
    return columnNames_;
  }

  void Blueprint::AddColumn(Column&& col) {
    columns_.emplace(col.name, std::move(col));
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
