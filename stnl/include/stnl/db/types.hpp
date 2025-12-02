#ifndef STNL_DB_TYPES_HPP
#define STNL_DB_TYPES_HPP

namespace STNL {

  enum SQLDataType {
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

}

#endif // STNL_DB_TYPES_HPP