
#include "stnl/column.hpp"
#include "stnl/utils.hpp"


namespace STNL
{
  Column::Column(std::string colRealName, ColumnType colType)
    : realName(colRealName), 
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

  CharProxy::CharProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Char;
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

  DateProxy::DateProxy(Column &c) : ColumnProxy(c) {
    col.type = ColumnType::Date;
  }

  TimestampProxy::TimestampProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Timestamp;
  }

  TimestampProxy& TimestampProxy::Precision(unsigned short v) {
    col.precision = v;
    return *this;
  }

  UUIDProxy::UUIDProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::UUID;
  }

  TextProxy::TextProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Text;
  }
  
} // namespace STNL
