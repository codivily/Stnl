#ifndef STNL_SP_PARAM_HPP
#define STNL_SP_PARAM_HPP

#include "types.hpp"
#include <map>
#include <string>
#include <vector>

namespace STNL {

struct SrParam {
    std::string name;
    SQLDataType type;
    size_t length;
    bool nullable;
    bool out;
    bool in;
    unsigned short precision;
    unsigned short scale;
    std::string defaultValue;
    explicit SrParam(std::string paramName, SQLDataType const &paramType);
};

template <typename Derived>
class SrParamProxy {
  protected:
    SrParam &param;
    explicit SrParamProxy(SrParam &p) : param(p) {}

  public:
    Derived &Name(std::string const &name) {
        param.name = name;
        return static_cast<Derived &>(*this);
    }

    Derived &Null() {
        param.nullable = true;
        return static_cast<Derived &>(*this);
    }

    Derived &NotNull() {
        param.nullable = false;
        return static_cast<Derived &>(*this);
    }

    Derived &Out(bool v = true) {
        param.out = v;
        return static_cast<Derived &>(*this);
    }

    Derived &In(bool v = true) {
        param.in = v;
        return static_cast<Derived &>(*this);
    }

    Derived &InOut() {
        param.in = true;
        param.out = true;
        return static_cast<Derived &>(*this);
    }

    virtual Derived &Default(std::string const &v) {
        param.defaultValue = std::string(v);
        return static_cast<Derived &>(*this);
    }
};

class BigIntParamProxy : public SrParamProxy<BigIntParamProxy> {
  public:
    BigIntParamProxy(SrParam &param);
    BigIntParamProxy &Identity(bool v = true);
};

class IntegerParamProxy : public SrParamProxy<IntegerParamProxy> {
  public:
    IntegerParamProxy(SrParam &param);
    IntegerParamProxy &Identity(bool const &v = true);
};

class SmallIntParamProxy : public SrParamProxy<SmallIntParamProxy> {
  public:
    SmallIntParamProxy(SrParam &param);
};

class NumericParamProxy : public SrParamProxy<NumericParamProxy> {
  public:
    NumericParamProxy(SrParam &param);
    NumericParamProxy &Precision(unsigned short v);
    NumericParamProxy &Scale(unsigned short v);
};

class BitParamProxy : public SrParamProxy<BitParamProxy> {
  public:
    BitParamProxy(SrParam &param);
    BitParamProxy &N(unsigned short v);
    BitParamProxy &Default(std::string const &v = "'1'") override;
};

class CharParamProxy : public SrParamProxy<CharParamProxy> {
  public:
    CharParamProxy(SrParam &param);
    CharParamProxy &Length(std::size_t v);
};

class VarcharParamProxy : public SrParamProxy<VarcharParamProxy> {
  public:
    VarcharParamProxy(SrParam &param);
    VarcharParamProxy &Length(std::size_t v);
};

class BooleanParamProxy : public SrParamProxy<BooleanParamProxy> {
  public:
    BooleanParamProxy(SrParam &param);
    BooleanParamProxy &Default(std::string const &v = "true") override;
};

class DateParamProxy : public SrParamProxy<DateParamProxy> {
  public:
    DateParamProxy(SrParam &param);
    DateParamProxy &Default(std::string const &v = "CURRENT_DATE") override;
};

class TimestampParamProxy : public SrParamProxy<TimestampParamProxy> {
  public:
    TimestampParamProxy(SrParam &param);
    TimestampParamProxy &Default(std::string const &v = "CURRENT_TIMESTAMP") override;
};

class UUIDParamProxy : public SrParamProxy<UUIDParamProxy> {
  public:
    UUIDParamProxy(SrParam &param);
    UUIDParamProxy &Default(std::string const &v = "uuidv7()") override;
};

class TextParamProxy : public SrParamProxy<TextParamProxy> {
  public:
    TextParamProxy(SrParam &param);
};
} // namespace STNL

#endif // STNL_SP_PARAM_HPP