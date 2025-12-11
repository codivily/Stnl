
#include "stnl/db/types.hpp"
#include "stnl/db/sp_param.hpp"
#include "stnl/core/utils.hpp"


namespace STNL
{
  SpParam::SpParam(std::string paraName, SQLDataType const& paramType)
    : name(paraName),
      type(paramType),
      length(0),
      nullable(false),
      precision(0),
      scale(0),
      defaultValue{},
      in(false),
      out(false) {}

  BigIntParamProxy::BigIntParamProxy(SpParam& param) : SpParamProxy(param) {
    param.type = SQLDataType::BigInt;
  }

  IntegerParamProxy::IntegerParamProxy(SpParam& param) : SpParamProxy(param) {
    param.type = SQLDataType::Integer;
  }

  SmallIntParamProxy::SmallIntParamProxy(SpParam& param) : SpParamProxy(param) {
    param.type = SQLDataType::SmallInt;
  }

  NumericParamProxy::NumericParamProxy(SpParam& param) : SpParamProxy(param) {
    param.type = SQLDataType::Numeric;
    param.precision = 9;
    param.scale = 0;
  }

  NumericParamProxy& NumericParamProxy::Precision(unsigned short v) {
    param.precision = v;
    return *this;
  }

  NumericParamProxy& NumericParamProxy::Scale(unsigned short v) {
    param.scale = v;
    return *this;
  }

  BitParamProxy::BitParamProxy(SpParam& param) : SpParamProxy(param) {
    param.type = SQLDataType::Bit;
    param.length = 1;
  }

  BitParamProxy& BitParamProxy::N(unsigned short v) {
    param.length = v;
    return *this;
  }

  BitParamProxy& BitParamProxy::Default(std::string const& v) {
    return SpParamProxy<BitParamProxy>::Default(v);
  }

  CharParamProxy::CharParamProxy(SpParam& param) : SpParamProxy(param) {
    param.type = SQLDataType::Char;
    param.length = 1;
  }

  CharParamProxy& CharParamProxy::Length(std::size_t v) {
    param.length = v;
    return *this;
  }

  VarcharParamProxy::VarcharParamProxy(SpParam& param) : SpParamProxy(param) {
    param.type = SQLDataType::Varchar;
    param.length = 255;
  }

  VarcharParamProxy& VarcharParamProxy::Length(std::size_t v) {
    param.length = v;
    return *this;
  }

  BooleanParamProxy::BooleanParamProxy(SpParam& param) : SpParamProxy(param) {
    param.type = SQLDataType::Boolean;
  }

  BooleanParamProxy& BooleanParamProxy::Default(std::string const& v) {
    return SpParamProxy<BooleanParamProxy>::Default(v);
  }

  DateParamProxy::DateParamProxy(SpParam &c) : SpParamProxy(param) {
    param.type = SQLDataType::Date;
  }

  DateParamProxy& DateParamProxy::Default(std::string const& v) {
    return SpParamProxy<DateParamProxy>::Default(v);
  }

  TimestampParamProxy::TimestampParamProxy(SpParam& param) : SpParamProxy(param) {
    param.type = SQLDataType::Timestamp;
  }

  TimestampParamProxy& TimestampParamProxy::Default(std::string const& v) {
    return SpParamProxy<TimestampParamProxy>::Default(v);
  }

  UUIDParamProxy::UUIDParamProxy(SpParam& param) : SpParamProxy(param) {
    param.type = SQLDataType::UUID;
  }

  UUIDParamProxy& UUIDParamProxy::Default(std::string const& v) {
    return SpParamProxy<UUIDParamProxy>::Default(v);
  }

  TextParamProxy::TextParamProxy(SpParam& param) : SpParamProxy(param) {
    param.type = SQLDataType::Text;
  }
  
} // namespace STNL
