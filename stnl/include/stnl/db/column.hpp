#ifndef STNL_COLUMN_HPP
#define STNL_COLUMN_HPP

#include "types.hpp"
#include <boost/optional.hpp>
#include <string>

namespace STNL {

struct Column {
    std::string tableName;
    std::string realName;
    std::string name;
    SQLDataType type;
    std::size_t length;
    bool identity;
    bool index;
    bool unique;
    bool nullable;
    unsigned short precision;
    unsigned short scale;
    std::string defaultValue;
    Column(std::string tableName, const std::string &colRealName, SQLDataType colType);
};

template <typename Derived>
class ColumnProxy {
  protected:
    Column &col;
    explicit ColumnProxy(Column &c) : col(c) {}

  public:
    Derived &Name(std::string &name) {
        col.name = name;
        return static_cast<Derived &>(*this);
    }

    Derived &Null() {
        col.nullable = true;
        return static_cast<Derived &>(*this);
    }

    Derived &NotNull() {
        col.nullable = false;
        return static_cast<Derived &>(*this);
    }

    virtual Derived &Default(std::string const &v) {
        col.defaultValue = std::string{v};
        return static_cast<Derived &>(*this);
    }
};

class BigIntProxy : public ColumnProxy<BigIntProxy> {
  public:
    BigIntProxy(Column &c);
    BigIntProxy &Identity(bool v = true);
    void Index(bool const &v = true);
    void Unique(bool const &v = true);
};

class IntegerProxy : public ColumnProxy<IntegerProxy> {
  public:
    IntegerProxy(Column &c);
    IntegerProxy &Identity(bool const &v = true);
    void Index(bool const &v = true);
    void Unique(bool const &v = true);
};

class SmallIntProxy : public ColumnProxy<SmallIntProxy> {
  public:
    SmallIntProxy(Column &c);
};

class NumericProxy : public ColumnProxy<NumericProxy> {
  public:
    NumericProxy(Column &c);
    NumericProxy &Precision(unsigned short v);
    NumericProxy &Scale(unsigned short v);
};

class BitProxy : public ColumnProxy<BitProxy> {
  public:
    BitProxy(Column &c);
    BitProxy &N(unsigned short v);
    BitProxy &Default(std::string const &v = "'1'") override;
};

class CharProxy : public ColumnProxy<CharProxy> {
  public:
    CharProxy(Column &c);
    CharProxy &Length(std::size_t v);
    void Index(bool const &v = true);
    void Unique(bool const &v = true);
};

class VarcharProxy : public ColumnProxy<VarcharProxy> {
  public:
    VarcharProxy(Column &c);
    VarcharProxy &Length(std::size_t v);
    void Index(bool const &v = true);
    void Unique(bool const &v = true);
};

class BooleanProxy : public ColumnProxy<BooleanProxy> {
  public:
    BooleanProxy(Column &c);
    BooleanProxy &Default(std::string const &v = "true") override;
};

class DateProxy : public ColumnProxy<DateProxy> {
  public:
    DateProxy(Column &c);
    DateProxy &Default(std::string const &v = "CURRENT_DATE") override;
    void Index(bool const &v = true);
};

class TimestampProxy : public ColumnProxy<TimestampProxy> {
  public:
    TimestampProxy(Column &c);
    TimestampProxy &Default(std::string const &v = "CURRENT_TIMESTAMP") override;
    void Index(bool const &v = true);
};

class UUIDProxy : public ColumnProxy<UUIDProxy> {
  public:
    UUIDProxy(Column &c);
    UUIDProxy &Default(std::string const &v = "uuidv7()") override;
    void Index(bool const &v = true);
    void Unique(bool const &v = true);
};

class TextProxy : public ColumnProxy<TextProxy> {
  public:
    TextProxy(Column &c);
};

} // namespace STNL

#endif // STNL_COLUMN_HPP
