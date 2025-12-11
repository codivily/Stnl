#include "stnl/db/types.hpp"
#include "stnl/db/sr_blueprint.hpp"
#include "stnl/db/sr_param.hpp"
#include "stnl/core/utils.hpp"

#include <unordered_map>
#include <vector>
#include <string>

namespace STNL {

  SrBlueprint::SrBlueprint(std::string const spName) : spName_(spName) {}

  std::string const& SrBlueprint::GetName() const { return spName_; }
  std::unordered_map<std::string, SrParam> const& SrBlueprint::GetParams() const { return spParams_; }

  SrParam& SrBlueprint::GetOrAddParam(std::string paramName) {
    std::string name = Utils::StringToLower(paramName);
    auto it = spParams_.find(name);
    if (it == spParams_.end()) {
      auto [newIt, inserted] = spParams_.emplace(name, SrParam(paramName, SQLDataType::Undefined));
      if (inserted) { spParamNames_.emplace_back(name); }
      return newIt->second;
    }
    return it->second;
  }
  
  std::string& SrBlueprint::Body() {
    return spBody_;
  }

  std::string const& SrBlueprint::GetBody() const {
    return spBody_;
  }

  std::string& SrBlueprint::BodyDelimiter() {
    return spBodyDelimiter_;
  }
  
  std::string const& SrBlueprint::GetBodyDelimiter() const {
    return spBodyDelimiter_;
  }

  std::vector<std::string> const& SrBlueprint::GetParamNames() const {
    return spParamNames_;
  }

  BigIntParamProxy SrBlueprint::BigInt(std::string name) {
    SrParam& param = GetOrAddParam(std::move(name));
    return BigIntParamProxy{param};
  }

  IntegerParamProxy SrBlueprint::Integer(std::string name) {
    SrParam& param = GetOrAddParam(std::move(name));
    return IntegerParamProxy{param};
  }

  SmallIntParamProxy SrBlueprint::SmallInt(std::string name) {
    SrParam& param = GetOrAddParam(std::move(name));
    return SmallIntParamProxy{param};
  }

  NumericParamProxy SrBlueprint::Numeric(std::string name) {
    SrParam& param = GetOrAddParam(std::move(name));
    return NumericParamProxy{param};
  }

  BitParamProxy SrBlueprint::Bit(std::string name) {
    SrParam& param = GetOrAddParam(std::move(name));
    return BitParamProxy{param};
  }

  CharParamProxy SrBlueprint::Char(std::string name) {
    SrParam& param = GetOrAddParam(name);
    return CharParamProxy{param};
  }

  VarcharParamProxy SrBlueprint::Varchar(std::string name) {
    SrParam& param = GetOrAddParam(std::move(name));
    return VarcharParamProxy{param};
  }

  BooleanParamProxy SrBlueprint::Boolean(std::string name) {
    SrParam& param = GetOrAddParam(std::move(name));
    return BooleanParamProxy{param};
  }

  DateParamProxy SrBlueprint::Date(std::string name) {
    SrParam& param = GetOrAddParam(std::move(name));
    return DateParamProxy{param};
  }

  TimestampParamProxy SrBlueprint::Timestamp(std::string name) {
    SrParam& param = GetOrAddParam(std::move(name));
    return TimestampParamProxy{param};
  }

  UUIDParamProxy SrBlueprint::UUID(std::string name) {
    SrParam& param = GetOrAddParam(std::move(name));
    return UUIDParamProxy{param};
  }

  TextParamProxy SrBlueprint::Text(std::string name) {
    SrParam& param = GetOrAddParam(std::move(name));
    return TextParamProxy(param);
  }
}