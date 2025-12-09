#include "stnl/db/types.hpp"
#include "stnl/db/sp_blueprint.hpp"
#include "stnl/db/sp_param.hpp"
#include "stnl/core/utils.hpp"

#include <unordered_map>
#include <vector>
#include <string>

namespace STNL {

  SpBlueprint::SpBlueprint(std::string const spName) : spName_(spName) {}

  std::string const& SpBlueprint::GetName() const { return spName_; }
  std::unordered_map<std::string, SpParam> const& SpBlueprint::GetParams() const { return spParams_; }

  SpParam& SpBlueprint::GetOrAddParam(std::string paramName) {
    std::string name = Utils::StringToLower(paramName);
    auto it = spParams_.find(name);
    if (it == spParams_.end()) {
      auto [newIt, inserted] = spParams_.emplace(name, SpParam(paramName, SQLDataType::Undefined));
      if (inserted) { spParamNames_.emplace_back(name); }
      return newIt->second;
    }
    return it->second;
  }
  
  std::string& SpBlueprint::Body() {
    return spBody_;
  }

  std::string const& SpBlueprint::GetBody() const {
    return spBody_;
  }

  std::vector<std::string> const& SpBlueprint::GetParamNames() const {
    return spParamNames_;
  }

  BigIntParamProxy SpBlueprint::BigInt(std::string name) {
    SpParam& param = GetOrAddParam(std::move(name));
    return BigIntParamProxy{param};
  }

  IntegerParamProxy SpBlueprint::Integer(std::string name) {
    SpParam& param = GetOrAddParam(std::move(name));
    return IntegerParamProxy{param};
  }

  SmallIntParamProxy SpBlueprint::SmallInt(std::string name) {
    SpParam& param = GetOrAddParam(std::move(name));
    return SmallIntParamProxy{param};
  }

  NumericParamProxy SpBlueprint::Numeric(std::string name) {
    SpParam& param = GetOrAddParam(std::move(name));
    return NumericParamProxy{param};
  }

  BitParamProxy SpBlueprint::Bit(std::string name) {
    SpParam& param = GetOrAddParam(std::move(name));
    return BitParamProxy{param};
  }

  CharParamProxy SpBlueprint::Char(std::string name) {
    SpParam& param = GetOrAddParam(name);
    return CharParamProxy{param};
  }

  VarcharParamProxy SpBlueprint::Varchar(std::string name) {
    SpParam& param = GetOrAddParam(std::move(name));
    return VarcharParamProxy{param};
  }

  BooleanParamProxy SpBlueprint::Boolean(std::string name) {
    SpParam& param = GetOrAddParam(std::move(name));
    return BooleanParamProxy{param};
  }

  DateParamProxy SpBlueprint::Date(std::string name) {
    SpParam& param = GetOrAddParam(std::move(name));
    return DateParamProxy{param};
  }

  TimestampParamProxy SpBlueprint::Timestamp(std::string name) {
    SpParam& param = GetOrAddParam(std::move(name));
    return TimestampParamProxy{param};
  }

  UUIDParamProxy SpBlueprint::UUID(std::string name) {
    SpParam& param = GetOrAddParam(std::move(name));
    return UUIDParamProxy{param};
  }

  TextParamProxy SpBlueprint::Text(std::string name) {
    SpParam& param = GetOrAddParam(std::move(name));
    return TextParamProxy(param);
  }
}