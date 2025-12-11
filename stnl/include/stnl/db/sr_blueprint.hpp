#ifndef STNL_SP_BLUEPRINT
#define STNL_SP_BLUEPRINT

#include "sr_param.hpp"

#include <string>
#include <vector>
#include <unordered_map>

namespace STNL {
  class SrBlueprint {
    public:
      explicit SrBlueprint(const std::string tableName);
      virtual ~SrBlueprint() = default;
      std::string const& GetName() const;
      std::unordered_map<std::string, SrParam> const& GetParams() const;
      std::vector<std::string> const& GetParamNames() const;

      std::string& Body();
      std::string const& GetBody() const;
      std::string& BodyDelimiter();
      std::string const& GetBodyDelimiter() const;

      BigIntParamProxy BigInt(std::string name);
      IntegerParamProxy Integer(std::string name);
      SmallIntParamProxy SmallInt(std::string name);
      NumericParamProxy Numeric(std::string name);
      BitParamProxy Bit(std::string name);
      CharParamProxy Char(std::string name);
      VarcharParamProxy Varchar(std::string name);
      BooleanParamProxy Boolean(std::string name);
      DateParamProxy Date(std::string name);
      TimestampParamProxy Timestamp(std::string name);
      UUIDParamProxy UUID(std::string name);
      TextParamProxy Text(std::string name);

    private:
      std::string spName_;
      std::unordered_map<std::string, SrParam> spParams_;
      std::vector<std::string> spParamNames_;
      SrParam& GetOrAddParam(std::string paraName);

      std::string spBody_;
      std::string spBodyDelimiter_ = "$$";
  };
}

#endif // STNL_SP_BLUEPRINT