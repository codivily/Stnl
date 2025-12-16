#ifndef STNL_DB_BLUEPRINT_HPP
#define STNL_DB_BLUEPRINT_HPP

#include "stnl/db/column.hpp"

#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>

namespace STNL {
class BigIntProxy;    // Forward declaration
class IntegerProxy;   // Forward declaration
class SmallIntProxy;  // Forward declaration
class NumericProxy;   // Forward declaration
class BitProxy;       // Forward declaration
class CharProxy;      // Forward declaration
class VarcharProxy;   // Forward declaration
class BooleanProxy;   // Forward declaration
class DateProxy;      // Forward declaration
class TimestampProxy; // Forward declaration
class UUIDProxy;      // Forward declaration
class TextProxy;      // Forward declaration

class Blueprint {
  public:
    explicit Blueprint(std::string tableName);
    auto GetTableName() const -> std::string const &;
    auto GetColumns() const -> std::unordered_map<std::string, Column> const &;
    auto GetColumnNames() const -> std::vector<std::string> const &;

    auto BigInt(const std::string &name) -> BigIntProxy;
    auto Integer(const std::string &name) -> IntegerProxy;
    auto SmallInt(const std::string &name) -> SmallIntProxy;
    auto Numeric(const std::string &name) -> NumericProxy;
    auto Bit(const std::string &name) -> BitProxy;
    auto Char(const std::string &name) -> CharProxy;
    auto Varchar(const std::string &name) -> VarcharProxy;
    auto Boolean(const std::string &name) -> BooleanProxy;
    auto Date(const std::string &name) -> DateProxy;
    auto Timestamp(const std::string &name) -> TimestampProxy;
    auto UUID(const std::string &name) -> UUIDProxy;
    auto Text(const std::string &name) -> TextProxy;

    void AddColumn(Column &&col);

  private:
    std::string tableName_;
    std::unordered_map<std::string, Column> columns_;
    std::vector<std::string> columnNames_;

    Column &GetOrAddColumn(const std::string &realName);
};
} // namespace STNL

#endif // STNL_DB_BLUEPRINT_HPP
