
#include "stnl/db/column.hpp"
#include "stnl/core/utils.hpp"


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
      index(false),
      unique(false),
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

  void BigIntProxy::Index(bool const& v) {
    col.index = v;
  }

  void BigIntProxy::Unique(bool const& v) {
    col.unique = v;
  }

  IntegerProxy::IntegerProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Integer;
  }

  IntegerProxy& IntegerProxy::Identity(bool const& v) {
    col.identity = v;
    return *this;
  }

  void IntegerProxy::Index(bool const& v) {
    col.index = v;
  }

  void IntegerProxy::Unique(bool const& v) {
    col.unique = v;
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

  void CharProxy::Index(bool const& v) {
    col.index = v;
  }

  void CharProxy::Unique(bool const& v) {
    col.unique = v;
  }

  VarcharProxy::VarcharProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Varchar;
    col.length = 255;
  }

  VarcharProxy& VarcharProxy::Length(std::size_t v) {
    col.length = v;
    return *this;
  }

  void VarcharProxy::Index(bool const& v) {
    col.index = v;
  }

  void VarcharProxy::Unique(bool const& v) {
    col.unique = v;
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

  void DateProxy::Index(bool const& v) {
    col.index = v;
  }

  TimestampProxy::TimestampProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Timestamp;
  }

  TimestampProxy& TimestampProxy::Default(std::string const& v) {
    return ColumnProxy<TimestampProxy>::Default(v);
  }

  void TimestampProxy::Index(bool const& v) {
    col.index = v;
  }

  UUIDProxy::UUIDProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::UUID;
  }

  UUIDProxy& UUIDProxy::Default(std::string const& v) {
    return ColumnProxy<UUIDProxy>::Default(v);
  }

  void UUIDProxy::Index(bool const& v) {
    col.index = v;
  }

  void UUIDProxy::Unique(bool const& v) {
    col.unique = v;
  }

  TextProxy::TextProxy(Column& c) : ColumnProxy(c) {
    col.type = ColumnType::Text;
  }
  
} // namespace STNL
