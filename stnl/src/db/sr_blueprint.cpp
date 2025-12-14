#include "stnl/db/sr_blueprint.hpp"
#include "stnl/core/utils.hpp"
#include "stnl/db/sr_param.hpp"
#include "stnl/db/types.hpp"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace STNL {

SrBlueprint::SrBlueprint(std::string spName) : spName_(std::move(spName)) {}

auto SrBlueprint::GetName() const -> std::string const & {
    return spName_;
}
auto SrBlueprint::GetParams() const -> std::unordered_map<std::string, SrParam> const & {
    return spParams_;
}

auto SrBlueprint::GetOrAddParam(const std::string &paramName) -> SrParam & {
    std::string name = Utils::StringToLower(paramName);
    auto it = spParams_.find(name);
    if (it == spParams_.end()) {
        auto [newIt, inserted] = spParams_.emplace(name, SrParam(paramName, SQLDataType::Undefined));
        if (inserted) { spParamNames_.emplace_back(name); }
        return newIt->second;
    }
    return it->second;
}

auto SrBlueprint::Body() -> std::string & {
    return spBody_;
}

auto SrBlueprint::GetBody() const -> std::string const & {
    return spBody_;
}

auto SrBlueprint::BodyDelimiter() -> std::string & {
    return spBodyDelimiter_;
}

auto SrBlueprint::GetBodyDelimiter() const -> std::string const & {
    return spBodyDelimiter_;
}

auto SrBlueprint::GetParamNames() const -> std::vector<std::string> const & {
    return spParamNames_;
}

auto SrBlueprint::BigInt(const std::string &name) -> BigIntParamProxy {
    SrParam &param = GetOrAddParam(name);
    return BigIntParamProxy{param};
}

auto SrBlueprint::Integer(const std::string &name) -> IntegerParamProxy {
    SrParam &param = GetOrAddParam(name);
    return IntegerParamProxy{param};
}

auto SrBlueprint::SmallInt(const std::string &name) -> SmallIntParamProxy {
    SrParam &param = GetOrAddParam(name);
    return SmallIntParamProxy{param};
}

auto SrBlueprint::Numeric(const std::string &name) -> NumericParamProxy {
    SrParam &param = GetOrAddParam(name);
    return NumericParamProxy{param};
}

auto SrBlueprint::Bit(const std::string &name) -> BitParamProxy {
    SrParam &param = GetOrAddParam(name);
    return BitParamProxy{param};
}

auto SrBlueprint::Char(const std::string &name) -> CharParamProxy {
    SrParam &param = GetOrAddParam(name);
    return CharParamProxy{param};
}

auto SrBlueprint::Varchar(const std::string &name) -> VarcharParamProxy {
    SrParam &param = GetOrAddParam(name);
    return VarcharParamProxy{param};
}

auto SrBlueprint::Boolean(const std::string &name) -> BooleanParamProxy {
    SrParam &param = GetOrAddParam(name);
    return BooleanParamProxy{param};
}

auto SrBlueprint::Date(const std::string &name) -> DateParamProxy {
    SrParam &param = GetOrAddParam(name);
    return DateParamProxy{param};
}

auto SrBlueprint::Timestamp(const std::string &name) -> TimestampParamProxy {
    SrParam &param = GetOrAddParam(name);
    return TimestampParamProxy{param};
}

auto SrBlueprint::UUID(const std::string &name) -> UUIDParamProxy {
    SrParam &param = GetOrAddParam(name);
    return UUIDParamProxy{param};
}

auto SrBlueprint::Text(const std::string &name) -> TextParamProxy {
    SrParam &param = GetOrAddParam(name);
    return {param};
}
} // namespace STNL