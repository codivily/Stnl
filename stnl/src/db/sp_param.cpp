
#include "stnl/db/types.hpp"
#incude "stnl/db/sp_param.hpp"
#include "stnl/db/column.hpp"
#include "stnl/core/utils.hpp"


namespace STNL
{
  SpParam::SpParam(std::string paraName, SQLDataType& paramType)
    : name(paraName),
      type(paramType)
      length(0),
      nullable(false),
      precision(0),
      scale(0),
      defaultValue{} {}

  BigIntParamProxy::BigIntParamProxy(SpParam& c) : SpParamProxy(c) {
    col.type = SQLDataType::BigInt;
  }

  IntegerParamProxy::IntegerParamProxy(SpParam& c) : SpParamProxy(c) {
    col.type = SQLDataType::Integer;
  }

  SmallIntParamProxy::SmallIntParamProxy(SpParam& c) : SpParamProxy(c) {
    col.type = SQLDataType::SmallInt;
  }

  NumericParamProxy::NumericParamProxy(SpParam& c) : SpParamProxy(c) {
    col.type = SQLDataType::Numeric;
    col.precision = 9;
    col.scale = 0;
  }

  NumericParamProxy& NumericParamProxy::Precision(unsigned short v) {
    col.precision = v;
    return *this;
  }

  NumericParamProxy& NumericParamProxy::Scale(unsigned short v) {
    col.scale = v;
    return *this;
  }

  BitParamProxy::BitParamProxy(SpParam& c) : SpParamProxy(c) {
    col.type = SQLDataType::Bit;
    col.length = 1;
  }

  BitParamProxy& BitParamProxy::N(unsigned short v) {
    col.length = v;
    return *this;
  }

  BitParamProxy& BitParamProxy::Default(std::string const& v) {
    return SpParamProxy<BitParamProxy>::Default(v);
  }

  CharParamProxy::CharParamProxy(SpParam& c) : SpParamProxy(c) {
    col.type = SQLDataType::Char;
    col.length = 1;
  }

  CharParamProxy& CharParamProxy::Length(std::size_t v) {
    col.length = v;
    return *this;
  }

  VarcharParamProxy::VarcharParamProxy(SpParam& c) : SpParamProxy(c) {
    col.type = SQLDataType::Varchar;
    col.length = 255;
  }

  VarcharParamProxy& VarcharParamProxy::Length(std::size_t v) {
    col.length = v;
    return *this;
  }

  BooleanParamProxy::BooleanParamProxy(SpParam& c) : SpParamProxy(c) {
    col.type = SQLDataType::Boolean;
  }

  BooleanParamProxy& BooleanParamProxy::Default(std::string const& v) {
    return SpParamProxy<BooleanParamProxy>::Default(v);
  }

  DateParamProxy::DateParamProxy(SpParam &c) : SpParamProxy(c) {
    col.type = SQLDataType::Date;
  }

  DateParamProxy& DateParamProxy::Default(std::string const& v) {
    return SpParamProxy<DateParamProxy>::Default(v);
  }

  TimestampParamProxy::TimestampParamProxy(SpParam& c) : SpParamProxy(c) {
    col.type = SQLDataType::Timestamp;
  }

  TimestampParamProxy& TimestampParamProxy::Default(std::string const& v) {
    return SpParamProxy<TimestampParamProxy>::Default(v);
  }

  UUIDParamProxy::UUIDParamProxy(SpParam& c) : SpParamProxy(c) {
    col.type = SQLDataType::UUID;
  }

  UUIDParamProxy& UUIDParamProxy::Default(std::string const& v) {
    return SpParamProxy<UUIDParamProxy>::Default(v);
  }

  TextParamProxy::TextParamProxy(SpParam& c) : SpParamProxy(c) {
    col.type = SQLDataType::Text;
  }
  
} // namespace STNL
