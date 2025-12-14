
#include <utility>

#include "stnl/core/utils.hpp"
#include "stnl/db/column.hpp"
#include "stnl/db/types.hpp"

namespace STNL {
namespace {
constexpr unsigned short DEFAULT_NUMERIC_PRECISION = 9;
constexpr std::size_t DEFAULT_VARCHAR_LENGTH = 255;
constexpr std::size_t DEFAULT_CHAR_LENGTH = 1;
constexpr std::size_t DEFAULT_BIT_LENGTH = 1;
} // namespace
Column::Column(std::string colTableName, const std::string &colRealName, SQLDataType colType)
    : tableName(std::move(std::move(colTableName))), realName(colRealName), type(colType), length(0), identity(false), index(false), unique(false),
      nullable(false), precision(0), scale(0) {
    name = Utils::StringToLower(colRealName);
}

BigIntProxy::BigIntProxy(Column &c) : ColumnProxy(c) {
    col.type = SQLDataType::BigInt;
}

auto BigIntProxy::Identity(bool v) -> BigIntProxy & {
    col.identity = v;
    return *this;
}

void BigIntProxy::Index(bool const &v) {
    col.index = v;
}

void BigIntProxy::Unique(bool const &v) {
    col.unique = v;
}

IntegerProxy::IntegerProxy(Column &c) : ColumnProxy(c) {
    col.type = SQLDataType::Integer;
}

auto IntegerProxy::Identity(bool const &v) -> IntegerProxy & {
    col.identity = v;
    return *this;
}

void IntegerProxy::Index(bool const &v) {
    col.index = v;
}

void IntegerProxy::Unique(bool const &v) {
    col.unique = v;
}

SmallIntProxy::SmallIntProxy(Column &c) : ColumnProxy(c) {
    col.type = SQLDataType::SmallInt;
}

NumericProxy::NumericProxy(Column &c) : ColumnProxy(c) {
    col.type = SQLDataType::Numeric;
    col.precision = DEFAULT_NUMERIC_PRECISION;
    col.scale = 0;
}

auto NumericProxy::Precision(unsigned short v) -> NumericProxy & {
    col.precision = v;
    return *this;
}

auto NumericProxy::Scale(unsigned short v) -> NumericProxy & {
    col.scale = v;
    return *this;
}

BitProxy::BitProxy(Column &c) : ColumnProxy(c) {
    col.type = SQLDataType::Bit;
    col.length = DEFAULT_BIT_LENGTH;
}

auto BitProxy::N(unsigned short v) -> BitProxy & {
    col.length = v;
    return *this;
}

auto BitProxy::Default(std::string const &v) -> BitProxy & {
    return ColumnProxy<BitProxy>::Default(v);
}

CharProxy::CharProxy(Column &c) : ColumnProxy(c) {
    col.type = SQLDataType::Char;
    col.length = DEFAULT_CHAR_LENGTH;
}

auto CharProxy::Length(std::size_t v) -> CharProxy & {
    col.length = v;
    return *this;
}

void CharProxy::Index(bool const &v) {
    col.index = v;
}

void CharProxy::Unique(bool const &v) {
    col.unique = v;
}

VarcharProxy::VarcharProxy(Column &c) : ColumnProxy(c) {
    col.type = SQLDataType::Varchar;
    col.length = DEFAULT_VARCHAR_LENGTH;
}

auto VarcharProxy::Length(std::size_t v) -> VarcharProxy & {
    col.length = v;
    return *this;
}

void VarcharProxy::Index(bool const &v) {
    col.index = v;
}

void VarcharProxy::Unique(bool const &v) {
    col.unique = v;
}

BooleanProxy::BooleanProxy(Column &c) : ColumnProxy(c) {
    col.type = SQLDataType::Boolean;
}

auto BooleanProxy::Default(std::string const &v) -> BooleanProxy & {
    return ColumnProxy<BooleanProxy>::Default(v);
}

DateProxy::DateProxy(Column &c) : ColumnProxy(c) {
    col.type = SQLDataType::Date;
}

auto DateProxy::Default(std::string const &v) -> DateProxy & {
    return ColumnProxy<DateProxy>::Default(v);
}

void DateProxy::Index(bool const &v) {
    col.index = v;
}

TimestampProxy::TimestampProxy(Column &c) : ColumnProxy(c) {
    col.type = SQLDataType::Timestamp;
}

auto TimestampProxy::Default(std::string const &v) -> TimestampProxy & {
    return ColumnProxy<TimestampProxy>::Default(v);
}

void TimestampProxy::Index(bool const &v) {
    col.index = v;
}

UUIDProxy::UUIDProxy(Column &c) : ColumnProxy(c) {
    col.type = SQLDataType::UUID;
}

auto UUIDProxy::Default(std::string const &v) -> UUIDProxy & {
    return ColumnProxy<UUIDProxy>::Default(v);
}

void UUIDProxy::Index(bool const &v) {
    col.index = v;
}

void UUIDProxy::Unique(bool const &v) {
    col.unique = v;
}

TextProxy::TextProxy(Column &c) : ColumnProxy(c) {
    col.type = SQLDataType::Text;
}

} // namespace STNL
