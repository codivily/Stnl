#ifndef STNL_BLUEPRINT_HTPP
#define STNL_BLUEPRINT_HTPP

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
class TimestampProxy; // Forward declration
class UUIDProxy;      // Forward declaration
class TextProxy;      // Forward declaration

class Blueprint {
  public:
    explicit Blueprint(std::string tableName);
    virtual ~Blueprint() = default;
    std::string const &GetTableName() const;
    std::unordered_map<std::string, Column> const &GetColumns() const;
    std::vector<std::string> const &GetColumnNames() const;

    BigIntProxy BigInt(const std::string &name);
    IntegerProxy Integer(const std::string &name);
    SmallIntProxy SmallInt(const std::string &name);
    NumericProxy Numeric(const std::string &name);
    BitProxy Bit(const std::string &name);
    CharProxy Char(const std::string &name);
    VarcharProxy Varchar(const std::string &name);
    BooleanProxy Boolean(const std::string &name);
    DateProxy Date(const std::string &name);
    TimestampProxy Timestamp(const std::string &name);
    UUIDProxy UUID(const std::string &name);
    TextProxy Text(const std::string &name);

    void AddColumn(Column &&col);

  private:
    std::string tableName_;
    std::unordered_map<std::string, Column> columns_;
    std::vector<std::string> columnNames_;

    Column &GetOrAddColumn(const std::string &realName);
};
} // namespace STNL

#endif // STNL_BLUEPRINT_HTPP
