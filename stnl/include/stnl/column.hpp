#ifndef STNL_COLUMN_HPP
#define STNL_COLUMN_HPP

#include <boost/optional.hpp>
#include <string>

namespace STNL {

  enum ColumnType {
    Undefined,
    //
    BigInt,
    Integer,
    SmallInt,
    Numeric,
    Bit,
    Char,
    Varchar,
    Boolean,
    Date,
    Timestamp,
    UUID,
    Text
  };

  struct Column {
      std::string realName;
      std::string name;
      ColumnType type;
      std::size_t length;
      bool identity;
      bool nullable;
      unsigned short precision;
      unsigned short scale;
      std::string defaultValue;
      Column(std::string colRealName, ColumnType colType);
  };

  
  template<typename Derived>
  class ColumnProxy {
    protected:
      Column& col;
      explicit ColumnProxy(Column& c) : col(c) {}

    public:
      Derived& Name(std::string& name) {
        col.name = name;
        return static_cast<Derived&>(*this);
      }
      Derived& Nullable(bool v = true) {
        col.nullable = v;
        return static_cast<Derived&>(*this);
      }

      Derived& Default(std::string v) {
        col.defaultValue = v;
        return static_cast<Derived&>(*this);
      }
  };

  class BigIntProxy : public ColumnProxy<BigIntProxy> {
    public:
      BigIntProxy(Column& c);
      BigIntProxy& Identity(bool v = true);
  };

  class IntegerProxy : public ColumnProxy<IntegerProxy> {
    public: 
      IntegerProxy(Column& c);
      IntegerProxy& Identity(bool  = true);
  };

  class SmallIntProxy : public ColumnProxy<SmallIntProxy> {
    public:
      SmallIntProxy(Column& c);
  };

  class NumericProxy : public ColumnProxy<NumericProxy> {
    public:
      NumericProxy(Column& c);
      NumericProxy& Precision(unsigned short v);
      NumericProxy& Scale(unsigned short v);
  };

  class BitProxy : public ColumnProxy<BitProxy> {
    public:
      BitProxy(Column& c);
      BitProxy& N(unsigned short v);
  };

  class CharProxy : public ColumnProxy<CharProxy> {
    public:
      CharProxy(Column& c);
      CharProxy& Length(std::size_t v);
  };

  class VarcharProxy : public ColumnProxy<VarcharProxy> {
    public:
      VarcharProxy(Column& c);
      VarcharProxy& Length(std::size_t v);
  };

  class BooleanProxy : public ColumnProxy<BooleanProxy> {
    public:
      BooleanProxy(Column& c);
  };

  class DateProxy : public ColumnProxy<DateProxy> {
    public:
      DateProxy(Column& c);
  };

  class TimestampProxy : public ColumnProxy<TimestampProxy> {
    public:
      TimestampProxy(Column& c);
      TimestampProxy& Precision(unsigned short v);
  };

  class UUIDProxy : public ColumnProxy<UUIDProxy> {
    public:
      UUIDProxy(Column& c);
  };

  class TextProxy : public ColumnProxy<TextProxy> {
    public:
      TextProxy(Column& c);
  };

}

#endif // STNL_COLUMN_HPP