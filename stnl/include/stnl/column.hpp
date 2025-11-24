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
      std::string tableName;
      std::string realName;
      std::string name;
      ColumnType type;
      std::size_t length;
      bool identity;
      bool nullable;
      unsigned short precision;
      unsigned short scale;
      std::string defaultValue;
      Column(std::string tableName, std::string colRealName, ColumnType colType);
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
      
      Derived& Null() {
        col.nullable = true;
        return static_cast<Derived&>(*this);
      }

      Derived& NotNull() {
        col.nullable = false;
        return static_cast<Derived&>(*this);
      }

      virtual Derived& Default(std::string const& v) {
        col.defaultValue = std::string{v};
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
      IntegerProxy& Identity(bool v = true);
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
      BitProxy& Default(std::string const& v = "'1'::bit(1)") override;
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
      BooleanProxy& Default(std::string const& v = "true") override;
  };

  class DateProxy : public ColumnProxy<DateProxy> {
    public:
      DateProxy(Column& c);
      DateProxy& Default(std::string const& v = "CURRENT_DATE") override;
  };

  class TimestampProxy : public ColumnProxy<TimestampProxy> {
    public:
      TimestampProxy(Column& c);
      TimestampProxy& Default(std::string const& v = "CURRENT_TIMESTAMP") override;
  };

  class UUIDProxy : public ColumnProxy<UUIDProxy> {
    public:
      UUIDProxy(Column& c);
      UUIDProxy& Default(std::string const& v = "uuidv7()") override;
  };

  class TextProxy : public ColumnProxy<TextProxy> {
    public:
      TextProxy(Column& c);
  };

}

#endif // STNL_COLUMN_HPP