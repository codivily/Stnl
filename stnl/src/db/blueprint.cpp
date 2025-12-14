
#include "stnl/db/blueprint.hpp"
#include "stnl/core/utils.hpp"
#include "stnl/db/column.hpp"

#include <boost/optional.hpp>

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace STNL {

Blueprint::Blueprint(std::string tableName) : tableName_(std::move(tableName)) {}

auto Blueprint::GetTableName() const -> std::string const & {
    return tableName_;
}
auto Blueprint::GetColumns() const -> std::unordered_map<std::string, Column> const & {
    return columns_;
}

auto Blueprint::GetOrAddColumn(const std::string &realName) -> Column & {
    std::string name = Utils::StringToLower(realName);
    auto it = columns_.find(name);
    if (it == columns_.end()) {
        auto [newIt, inserted] = columns_.emplace(name, Column{this->tableName_, realName, SQLDataType::Undefined});
        if (inserted) { columnNames_.emplace_back(name); }
        return newIt->second;
    }
    return it->second;
}

auto Blueprint::GetColumnNames() const -> std::vector<std::string> const & {
    return columnNames_;
}

void Blueprint::AddColumn(Column &&col) {
    columns_.emplace(col.name, std::move(col));
}

auto Blueprint::BigInt(const std::string &name) -> BigIntProxy {
    Column &col = GetOrAddColumn(name);
    return BigIntProxy{col};
}

auto Blueprint::Integer(const std::string &name) -> IntegerProxy {
    Column &col = GetOrAddColumn(name);
    return IntegerProxy{col};
}

auto Blueprint::SmallInt(const std::string &name) -> SmallIntProxy {
    Column &col = GetOrAddColumn(name);
    return SmallIntProxy{col};
}

auto Blueprint::Numeric(const std::string &name) -> NumericProxy {
    Column &col = GetOrAddColumn(name);
    return NumericProxy{col};
}

auto Blueprint::Bit(const std::string &name) -> BitProxy {
    Column &col = GetOrAddColumn(name);
    return BitProxy{col};
}

auto Blueprint::Char(const std::string &name) -> CharProxy {
    Column &col = GetOrAddColumn(name);
    return CharProxy{col};
}

auto Blueprint::Varchar(const std::string &name) -> VarcharProxy {
    Column &col = GetOrAddColumn(name);
    return VarcharProxy{col};
}

auto Blueprint::Boolean(const std::string &name) -> BooleanProxy {
    Column &col = GetOrAddColumn(name);
    return BooleanProxy{col};
}

auto Blueprint::Date(const std::string &name) -> DateProxy {
    Column &col = GetOrAddColumn(name);
    return DateProxy{col};
}

auto Blueprint::Timestamp(const std::string &name) -> TimestampProxy {
    Column &col = GetOrAddColumn(name);
    return TimestampProxy{col};
}

auto Blueprint::UUID(const std::string &name) -> UUIDProxy {
    Column &col = GetOrAddColumn(name);
    return UUIDProxy{col};
}

auto Blueprint::Text(const std::string &name) -> TextProxy {
    Column &col = GetOrAddColumn(name);
    return {col};
}
} // namespace STNL
