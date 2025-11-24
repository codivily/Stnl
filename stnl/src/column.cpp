
#include "stnl/column.hpp"
#include "stnl/utils.hpp"


namespace STNL
{
  Column::Column(std::string colTableName, std::string colRealName, ColumnType colType)
    : tableName(colTableName),
      realName(colRealName), 
      type(std::move(colType)),
      length(0),
      identity(false),
      nullable(false),
      precision(0),
      scale(0),
      defaultValue{} {
        name = Utils::StringToLower(colRealName);
  }

  BigIntProxy::BigIntProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::BigInt;
  }

  BigIntProxy& BigIntProxy::Identity(bool v) {
    col.identity = v;
    return *this;
  }

  IntegerProxy::IntegerProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Integer;
  }

  IntegerProxy& IntegerProxy::Identity(bool v) {
    col.identity = v;
    return *this;
  }

  SmallIntProxy::SmallIntProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::SmallInt;
  }

  NumericProxy::NumericProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Numeric;
    col.precision = 9;
    col.scale = 0;
  }

  NumericProxy& NumericProxy::Precision(unsigned short v) {
    col.precision = v;
    return *this;
  }

  NumericProxy& NumericProxy::Scale(unsigned short v) {
    col.scale = v;
    return *this;
  }

  BitProxy::BitProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Bit;
    col.length = 1;
  }

  BitProxy& BitProxy::N(unsigned short v) {
    col.length = v;
    return *this;
  }

  BitProxy& BitProxy::Default(std::string const& v) {
    return ColumnProxy<BitProxy>::Default(v);
  }

  CharProxy::CharProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Char;
    col.length = 1;
  }

  CharProxy& CharProxy::Length(std::size_t v) {
    col.length = v;
    return *this;
  }

  VarcharProxy::VarcharProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Varchar;
    col.length = 255;
  }

  VarcharProxy& VarcharProxy::Length(std::size_t v) {
    col.length = v;
    return *this;
  }

  BooleanProxy::BooleanProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Boolean;
  }

  BooleanProxy& BooleanProxy::Default(std::string const& v) {
    return ColumnProxy<BooleanProxy>::Default(v);
  }

  DateProxy::DateProxy(Column &c) : ColumnProxy(c) {
    col.type = ColumnType::Date;
  }

  DateProxy& DateProxy::Default(std::string const& v) {
    return ColumnProxy<DateProxy>::Default(v);
  }

  TimestampProxy::TimestampProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Timestamp;
  }

  TimestampProxy& TimestampProxy::Default(std::string const& v) {
    return ColumnProxy<TimestampProxy>::Default(v);
  }

  UUIDProxy::UUIDProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::UUID;
  }

  UUIDProxy& UUIDProxy::Default(std::string const& v) {
    return ColumnProxy<UUIDProxy>::Default(v);
  }

  TextProxy::TextProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Text;
  }
  
} // namespace STNL
