#ifndef STNL_SP_BLUEPRINT
#define STNL_SP_BLUEPRINT

#include "sr_param.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace STNL {
class SrBlueprint {
  public:
    explicit SrBlueprint(std::string spName);
    virtual ~SrBlueprint() = default;
    std::string const &GetName() const;
    std::unordered_map<std::string, SrParam> const &GetParams() const;
    std::vector<std::string> const &GetParamNames() const;

    std::string &Body();
    std::string const &GetBody() const;
    std::string &BodyDelimiter();
    std::string const &GetBodyDelimiter() const;

    BigIntParamProxy BigInt(const std::string &name);
    IntegerParamProxy Integer(const std::string &name);
    SmallIntParamProxy SmallInt(const std::string &name);
    NumericParamProxy Numeric(const std::string &name);
    BitParamProxy Bit(const std::string &name);
    CharParamProxy Char(const std::string &name);
    VarcharParamProxy Varchar(const std::string &name);
    BooleanParamProxy Boolean(const std::string &name);
    DateParamProxy Date(const std::string &name);
    TimestampParamProxy Timestamp(const std::string &name);
    UUIDParamProxy UUID(const std::string &name);
    TextParamProxy Text(const std::string &name);

  private:
    std::string spName_;
    std::unordered_map<std::string, SrParam> spParams_;
    std::vector<std::string> spParamNames_;
    SrParam &GetOrAddParam(const std::string &paramName);

    std::string spBody_;
    std::string spBodyDelimiter_ = "$$";
};
} // namespace STNL

#endif // STNL_SP_BLUEPRINT