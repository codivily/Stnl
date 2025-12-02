#include "stnl/db/types.hpp"
#include "stnl/db/sp_blueprint.hpp"
#include "stnl/db/sp_param.hpp"
#include "stnl/core/utils.hpp"


#include <map>
#include <vector>
#include <string>


namespace STNL {

  SpBlueprint::SpBlueprint(std::string const spName) : spName_(spName) {}

  std::string const& SpBlueprint::GetName() const { return spName_; }
  std::unordered_map<std::string, SpParam> const& SpBlueprint::GetParams() const { return spParams_; }

  SpParam& SpBlueprint::GetOrAddSpParam(std::string paramName) {
    std::string name = Utils::StringToLower(paramName);
    auto it = spParams_.find(name);
    if (it == spParams_.end()) {
      auto [newIt, inserted] = spParams_.emplace(name, SpParam{std::move(paramName), SQLDataType::Undefined});
      if (inserted) { spParamNames_.emplace_back(name); }
      return newIt->second;
    }
    return it->second;
  }
  
  std::string& SpBlueprint::Body() {
    return spBody_;
  }

  std::vector<std::string> const& SpBlueprint::GetParamNames() const {
    return spParamNames_;
  }

  std::unordered_map<std::string, SpParam> const& SpBlueprint::GetParams() const {
    return spParams_;
  }

  void SpBlueprint::AddSpParam(SpParam&& param) {
    spParams_.emplace(param.name, std::move(param));
  }

  void SpBlueprint::AddSpParam(SpParam&& param) {
    spParams_.emplace(param.name, std::move(param));
  }

  BigIntParamProxy SpBlueprint::BigInt(std::string name) {
    SpParam& col = GetOrAddSpParam(std::move(name));
    return BigIntParamProxy{col};
  }

  IntegerParamProxy SpBlueprint::Integer(std::string name) {
    SpParam& col = GetOrAddSpParam(std::move(name));
    return IntegerParamProxy{col};
  }

  SmallIntParamProxy SpBlueprint::SmallInt(std::string name) {
    SpParam& col = GetOrAddSpParam(std::move(name));
    return SmallIntParamProxy{col};
  }

  NumericParamProxy SpBlueprint::Numeric(std::string name) {
    SpParam& col = GetOrAddSpParam(std::move(name));
    return NumericParamProxy{col};
  }

  BitParamProxy SpBlueprint::Bit(std::string name) {
    SpParam& col = GetOrAddSpParam(std::move(name));
    return BitParamProxy{col};
  }

  CharParamProxy SpBlueprint::Char(std::string name) {
    SpParam& col = GetOrAddSpParam(std::move(name));
    return CharParamProxy{col};
  }

  VarcharParamProxy SpBlueprint::Varchar(std::string name) {
    SpParam& col = GetOrAddSpParam(std::move(name));
    return VarcharParamProxy{col};
  }

  BooleanParamProxy SpBlueprint::Boolean(std::string name) {
    SpParam& col = GetOrAddSpParam(std::move(name));
    return BooleanParamProxy{col};
  }

  DateParamProxy SpBlueprint::Date(std::string name) {
    SpParam& col = GetOrAddSpParam(std::move(name));
    return DateParamProxy{col};
  }

  TimestampParamProxy SpBlueprint::Timestamp(std::string name) {
    SpParam& col = GetOrAddSpParam(std::move(name));
    return TimestampParamProxy{col};
  }

  UUIDParamProxy SpBlueprint::UUID(std::string name) {
    SpParam& col = GetOrAddSpParam(std::move(name));
    return UUIDParamProxy{col};
  }

  TextParamProxy SpBlueprint::Text(std::string name) {
    SpParam& col = GetOrAddSpParam(std::move(name));
    return TextParamProxy(col);
  }
}