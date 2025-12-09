#ifndef STNL_SP_PARAM_HPP
#define STNL_SP_PARAM_HPP

#include "types.hpp"
#include <string>
#include <vector>
#include <map>


namespace STNL {

  struct SpParam {
    std::string name;
    SQLDataType type;
    size_t length;
    bool nullable;
    bool out;
    bool in;
    unsigned short precision;
    unsigned short scale;
    std::string defaultValue;
    explicit SpParam(std::string paramName, SQLDataType const& paramType);
  };


  template <typename Derived>
  class SpParamProxy {
    protected:
      SpParam& param;
      explicit SpParamProxy(SpParam& p) : param(p) {}

    public:
      Derived& Name(std::string const& name) {
        param.name = name;
        return static_cast<Derived&>(*this);
      }

      Derived& Null() {
        param.nullable = true;
        return static_cast<Derived&>(*this);
      }

      Derived& NotNull() {
        param.nullable = false;
        return static_cast<Derived&>(*this);
      }

      Derived& Out(bool v = true) {
        param.out = v;
        return static_cast<Derived&>(*this);
      }

      Derived& In(bool v = true) {
        param.in = v;
        return static_cast<Derived&>(*this);
      }

      Derived& InOut() {
        param.in = true;
        param.out = true;
        return static_cast<Derived&>(*this);
      }

      virtual Derived& Default(std::string const& v) {
        param.defaultValue = std::string(v);
        return static_cast<Derived&>(*this);
      }
  };


  class BigIntParamProxy : public SpParamProxy<BigIntParamProxy> {
    public:
      BigIntParamProxy(SpParam& c);
      BigIntParamProxy& Identity(bool v = true);
  };

  class IntegerParamProxy : public SpParamProxy<IntegerParamProxy> {
    public: 
      IntegerParamProxy(SpParam& c);
      IntegerParamProxy& Identity(bool const &v = true);
  };

  class SmallIntParamProxy : public SpParamProxy<SmallIntParamProxy> {
    public:
      SmallIntParamProxy(SpParam& c);
  };

  class NumericParamProxy : public SpParamProxy<NumericParamProxy> {
    public:
      NumericParamProxy(SpParam& c);
      NumericParamProxy& Precision(unsigned short v);
      NumericParamProxy& Scale(unsigned short v);
  };

  class BitParamProxy : public SpParamProxy<BitParamProxy> {
    public:
      BitParamProxy(SpParam& c);
      BitParamProxy& N(unsigned short v);
      BitParamProxy& Default(std::string const& v = "'1'") override;
  };

  class CharParamProxy : public SpParamProxy<CharParamProxy> {
    public:
      CharParamProxy(SpParam& c);
      CharParamProxy& Length(std::size_t v);
  };

  class VarcharParamProxy : public SpParamProxy<VarcharParamProxy> {
    public:
      VarcharParamProxy(SpParam& c);
      VarcharParamProxy& Length(std::size_t v);
  };

  class BooleanParamProxy : public SpParamProxy<BooleanParamProxy> {
    public:
      BooleanParamProxy(SpParam& c);
      BooleanParamProxy& Default(std::string const& v = "true") override;
  };

  class DateParamProxy : public SpParamProxy<DateParamProxy> {
    public:
      DateParamProxy(SpParam& c);
      DateParamProxy& Default(std::string const& v = "CURRENT_DATE") override;
  };

  class TimestampParamProxy : public SpParamProxy<TimestampParamProxy> {
    public:
      TimestampParamProxy(SpParam& c);
      TimestampParamProxy& Default(std::string const& v = "CURRENT_TIMESTAMP") override;
  };

  class UUIDParamProxy : public SpParamProxy<UUIDParamProxy> {
    public:
      UUIDParamProxy(SpParam& c);
      UUIDParamProxy& Default(std::string const& v = "uuidv7()") override;
  };

  class TextParamProxy : public SpParamProxy<TextParamProxy> {
    public:
      TextParamProxy(SpParam& c);
  };
}

#endif // STNL_SP_PARAM_HPP