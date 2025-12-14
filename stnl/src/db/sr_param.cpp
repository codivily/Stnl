
#include <utility>

#include "stnl/core/utils.hpp"
#include "stnl/db/sr_param.hpp"
#include "stnl/db/types.hpp"

namespace {
constexpr unsigned short DEFAULT_NUMERIC_PRECISION = 9;
constexpr std::size_t DEFAULT_VARCHAR_LENGTH = 255;
} // namespace

namespace STNL {
SrParam::SrParam(std::string paramName, SQLDataType const &paramType)
    : name(std::move(paramName)), type(paramType), length(0), nullable(false), out(false), in(false), precision(0), scale(0) {}

BigIntParamProxy::BigIntParamProxy(SrParam &param) : SrParamProxy(param) {
    param.type = SQLDataType::BigInt;
}

IntegerParamProxy::IntegerParamProxy(SrParam &param) : SrParamProxy(param) {
    param.type = SQLDataType::Integer;
}

SmallIntParamProxy::SmallIntParamProxy(SrParam &param) : SrParamProxy(param) {
    param.type = SQLDataType::SmallInt;
}

NumericParamProxy::NumericParamProxy(SrParam &param) : SrParamProxy(param) {
    param.type = SQLDataType::Numeric;
    param.precision = DEFAULT_NUMERIC_PRECISION;
    param.scale = 0;
}

auto NumericParamProxy::Precision(unsigned short v) -> NumericParamProxy & {
    param.precision = v;
    return *this;
}

auto NumericParamProxy::Scale(unsigned short v) -> NumericParamProxy & {
    param.scale = v;
    return *this;
}

BitParamProxy::BitParamProxy(SrParam &param) : SrParamProxy(param) {
    param.type = SQLDataType::Bit;
    param.length = 1;
}

auto BitParamProxy::N(unsigned short v) -> BitParamProxy & {
    param.length = v;
    return *this;
}

auto BitParamProxy::Default(std::string const &v) -> BitParamProxy & {
    return SrParamProxy<BitParamProxy>::Default(v);
}

CharParamProxy::CharParamProxy(SrParam &param) : SrParamProxy(param) {
    param.type = SQLDataType::Char;
    param.length = 1;
}

auto CharParamProxy::Length(std::size_t v) -> CharParamProxy & {
    param.length = v;
    return *this;
}

VarcharParamProxy::VarcharParamProxy(SrParam &param) : SrParamProxy(param) {
    param.type = SQLDataType::Varchar;
    param.length = DEFAULT_VARCHAR_LENGTH;
}

auto VarcharParamProxy::Length(std::size_t v) -> VarcharParamProxy & {
    param.length = v;
    return *this;
}

BooleanParamProxy::BooleanParamProxy(SrParam &param) : SrParamProxy(param) {
    param.type = SQLDataType::Boolean;
}

auto BooleanParamProxy::Default(std::string const &v) -> BooleanParamProxy & {
    return SrParamProxy<BooleanParamProxy>::Default(v);
}

DateParamProxy::DateParamProxy(SrParam &param) : SrParamProxy(param) {
    param.type = SQLDataType::Date;
}

auto DateParamProxy::Default(std::string const &v) -> DateParamProxy & {
    return SrParamProxy<DateParamProxy>::Default(v);
}

TimestampParamProxy::TimestampParamProxy(SrParam &param) : SrParamProxy(param) {
    param.type = SQLDataType::Timestamp;
}

auto TimestampParamProxy::Default(std::string const &v) -> TimestampParamProxy & {
    return SrParamProxy<TimestampParamProxy>::Default(v);
}

UUIDParamProxy::UUIDParamProxy(SrParam &param) : SrParamProxy(param) {
    param.type = SQLDataType::UUID;
}

auto UUIDParamProxy::Default(std::string const &v) -> UUIDParamProxy & {
    return SrParamProxy<UUIDParamProxy>::Default(v);
}

TextParamProxy::TextParamProxy(SrParam &param) : SrParamProxy(param) {
    param.type = SQLDataType::Text;
}

} // namespace STNL
