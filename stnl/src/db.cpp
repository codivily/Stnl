

#include "stnl/db.hpp"
#include "stnl/blueprint.hpp"
#include "stnl/connection_pool.hpp"
#include "stnl/logger.hpp"
#include "stnl/utils.hpp"

#include <boost/asio.hpp>
#include <boost/json.hpp>

#include <pqxx/pqxx>
#include <pqxx/result>
#include <pqxx/field.hxx>

#include <format>
#include <string>
#include <format>
#include <thread>
#include <memory>
#include <future>
#include <map>
#include <iostream>
#include <sstream>

namespace asio = boost::asio;
namespace json = boost::json;

namespace STNL {

  std::string SelectedFieldIndex::Field(std::string const& fieldName) {
    fieldToIndex_[std::string{fieldName}] = fieldToIndex_.size();
    return std::string{fieldName};
  }

  std::string SelectedFieldIndex::operator()(std::string const& fieldName) {
    return this->Field(fieldName);
  }

  unsigned int SelectedFieldIndex::Index(std::string const& fieldName) const {
    auto it = fieldToIndex_.find(fieldName);
    if (it == fieldToIndex_.end()) {
      std::string errMsg =  std::format("SelectedFieldIndex::Index: field with name '{}' not found", fieldName);
      Logger::Err() << errMsg;
      throw std::runtime_error(errMsg);
    }
    return it->second;
  }

  const std::unordered_map<std::string, unsigned int>& SelectedFieldIndex::GetMap() const {
    return fieldToIndex_;
  }

  Inserter::Inserter(std::string const& tableName) : tableName_(tableName) {}
  
  std::string Inserter::GetSQLCmdName() const {
    return std::format("sql_cmd_insert_{}", tableName_);
  }

  std::string Inserter::GetSQLCmd() const { 
    return std::format("INSERT INTO {} ({}) VALUES ({})", tableName_, columnsSS_.str(), placeholderSS_.str());
  }

  pqxx::params& Inserter::GetParams() {
    return this->params_;
  }

  DB::DB(std::string& connStr, asio::io_context& ioc, size_t poolSize, size_t numThreads) : pool_(connStr, poolSize), ioc_(ioc), workGuard_(asio::make_work_guard(ioc_)) {
    /* there has to be at least one mandatory thread */
    if (numThreads == 0) { numThreads = 1; }
    for (size_t i = 0; i < numThreads; ++i) {
      threadPool_.emplace_back([this](){ this->ioc_.run(); });
    }
  }

  DB::~DB() {
    workGuard_.reset();
    for(auto& t : threadPool_) {
      if (t.joinable()) { t.join(); }
    }
  }

  asio::io_context& DB::GetIOC() {
    return ioc_;
  }

  QResult DB::Exec(std::string_view qSQL, bool silent) {
    if (!silent) { Logger::Dbg() << "DB::Exec:qSQL: \n" << qSQL; }
    QResult qResult{pqxx::result{}, false, ""};
    pqxx::connection* pConn = nullptr;
    try {
      pConn = pool_.GetConnection();
      pqxx::nontransaction tx(*pConn);
      pqxx::result r = tx.exec(qSQL);
      qResult.data = std::move(r);
      qResult.ok = true;
    } catch(std::exception& e) {
      qResult.ok = false;
      qResult.msg = Utils::Trim(std::string(e.what()));
      Logger::Err() << "DB::Exec: ErrorWhat: \n" << e.what();
      Logger::Err() << "DB::Exec: ErrorSQL: \n" << qSQL;
    }
    if (pConn) { pool_.ReturnConnection(pConn); }
    return qResult;
  }

  QResult DB::ExecSQLCmd(std::string const& sqlCmdName, std::string const& sqlCmd, pqxx::params& params, bool silent) {
    if (!silent) { Logger::Dbg() << std::format("DB::ExecSQLCmd:<{}>: {}", sqlCmdName, sqlCmd); }
    QResult qResult{pqxx::result{}, false, ""};
    pqxx::connection* pConn = nullptr;
    try {
      pConn = pool_.GetConnection();
      // pConn->prepare(sqlCmdName, sqlCmd);
      pqxx::nontransaction tx(*pConn);
      pqxx::result result = tx.exec(sqlCmd, params);
      qResult.data = result;
      qResult.ok = true;
    } catch(std::exception& e) {
      qResult.msg = Utils::Trim(std::string(e.what()));
      Logger::Err() << "DB::ExecSQLCmd: ErrorWhat: \n" << e.what();
      Logger::Err() << "DB::ExecSQLCmd: ErrorSQL: \n" << std::format("<{}>: {}", sqlCmdName, sqlCmd);
    }
    if (pConn) { pool_.ReturnConnection(pConn); }
    return qResult;
  }

  

  std::future<QResult> DB::ExecAsync(std::string_view qSQL, bool silent) {
    return Utils::AsFuture<QResult>(ioc_, [this, qSQL, silent = std::move(silent)]() { return this->Exec(qSQL, silent); });
  } 

  bool DB::TableExists(std::string_view tableName) {
    std::string qSQL = Utils::FixIndent(R"(
        SELECT 1
        FROM information_schema.tables
        WHERE LOWER(table_name) = LOWER(')" + pqxx::to_string(tableName) + R"(')
        AND table_schema = CURRENT_SCHEMA;
    )");
    QResult r = this->Exec(qSQL);
    if (!r.ok) {
      throw std::runtime_error("Failed to query table schema for " + std::string(tableName) + ": " + r.msg);
    }
    return (r.data.size() < 1 ? false : true);
  }

  std::vector<Column> DB::GetTableColumns(std::string_view tableName) {
      SelectedFieldIndex sfi;
      
      std::vector<std::string> select;
      select.emplace_back(sfi("table_name"));
      select.emplace_back(sfi("column_name"));
      select.emplace_back(sfi("data_type"));
      select.emplace_back(sfi("character_maximum_length"));
      select.emplace_back(sfi("numeric_precision"));
      select.emplace_back(sfi("numeric_scale"));
      select.emplace_back(sfi("is_nullable"));
      select.emplace_back(sfi("column_default"));
      select.emplace_back(sfi("identity_generation"));

      std::vector<std::string> where;
      where.emplace_back("table_schema = CURRENT_SCHEMA()");
      if (!tableName.empty()) { where.emplace_back(std::format("LOWER(table_name) = LOWER('{}')", tableName)); }
      std::string qSQL = std::format("SELECT {} FROM information_schema.columns WHERE {} ORDER BY table_name, ordinal_position", Utils::Join(select, ","), Utils::Join(where, " AND "));
      QResult r = this->Exec(qSQL);
      std::vector<Column> columns;
      
      if (!r.ok) {
          STNL::Logger::Err() << ("DB::GetTableColumns: Error executing query: " + r.msg);
          throw std::runtime_error("Failed to query table schema: " + r.msg);
      }

      // Iterate over results and map to Column structures
      for (const auto& row : r.data) {
          std::string table = row[sfi.Index("table_name")].as<std::string>();
          std::string colName = row[sfi.Index("column_name")].as<std::string>();
          std::string dataType = row[sfi.Index("data_type")].as<std::string>();
          std::string isNullable = row[sfi.Index("is_nullable")].as<std::string>();
          
          Column col(table, colName, ColumnType::Undefined); 

          // A. Handle Nullability
          col.nullable = (isNullable == "YES");

          // B. Handle Data Types and Attributes
          if (dataType == "bigint") {  col.type = ColumnType::BigInt; }
          else if (dataType == "integer") { col.type = ColumnType::Integer; }
          else if (dataType == "smallint") {  col.type = ColumnType::SmallInt;  }
          else if (dataType == "numeric") {
              col.type = ColumnType::Numeric;
              col.precision = row[sfi.Index("numeric_precision")].as<unsigned short>(0); 
              col.scale = row[sfi.Index("numeric_scale")].as<unsigned short>(0);     
          }
          else if (dataType == "bit") {
              col.type = ColumnType::Bit;
              col.length = row[sfi.Index("character_maximum_length")].as<std::size_t>(1);
          }
          else if (dataType == "character") {
              col.type = ColumnType::Char;
              col.length = row[sfi.Index("character_maximum_length")].as<std::size_t>(0);
          }
          else if (dataType == "character varying") {
              col.type = ColumnType::Varchar;
              col.length = row[sfi.Index("character_maximum_length")].as<std::size_t>(255);
          }
          else if (dataType == "boolean") { col.type = ColumnType::Boolean; }
          else if (dataType == "date") { col.type = ColumnType::Date; }
          else if (dataType.find("timestamp") != std::string::npos) { 
              col.type = ColumnType::Timestamp;
              col.length = row[sfi.Index("numeric_precision")].as<std::size_t>(6);
          }
          else if (dataType == "uuid") { col.type = ColumnType::UUID; }
          else if (dataType == "text") { col.type = ColumnType::Text; }
          else {
              STNL::Logger::Wrn() << ("DB::GetTableColumns: Unsupported data type: " + dataType + " for column " + colName);
              col.type = ColumnType::Undefined; 
          }

          // C. Handle IDENTITY 
          if (row[sfi.Index("identity_generation")].c_str() && row[sfi.Index("identity_generation")].c_str()[0] != '\0') {
              col.identity = true;
          }
          
          // D. Handle Default Value
          if (row[sfi.Index("column_default")].c_str() && row[sfi.Index("column_default")].c_str()[0] != '\0') {
              col.defaultValue = row[sfi.Index("column_default")].as<std::string>();
          }
          columns.push_back(std::move(col));
      }
      return columns;
  }

  Blueprint DB::QueryBlueprint(std::string_view tableName) {
    if (tableName.empty()){
      STNL::Logger::Err() << "DB::QueryBlueprint: Table name cannot be empty for Blueprint generation.";
      throw std::invalid_argument("Blueprint generation requires a non-empty table name.");
    }
    std::vector<Column> cols = GetTableColumns(tableName);
    Blueprint bp{std::string(tableName)};
    for (Column &col : cols) {
      bp.AddColumn(std::move(col));
    }
    return bp;
  }

  std::string DB::GetConnectionString(
      std::string_view dbName,
      std::string_view dbUser,
      std::string_view dbPassword,
      std::string_view dbHost,
      size_t dbPort,
      std::string_view dbSchema)
  {  
    return std::format("dbname={} user={} password={} host={} port={} options=-csearch_path={}",
      dbName, dbUser, dbPassword, dbHost, dbPort, dbSchema);
  }

  
  static boost::json::value row_to_json(pqxx::row const& row) {
    boost::json::object obj;
    for (const auto& field : row) {
        const char* name = field.name();
        if (field.is_null()) { 
            obj[name] = nullptr; 
        }
        else {
            switch (field.type())
            {
            case PgFieldTypeOID::int4:
            case PgFieldTypeOID::int8:
              obj[name] = field.as<long long>();
              break;
            case PgFieldTypeOID::numeric:
            case PgFieldTypeOID::float4:
            case PgFieldTypeOID::float8:
              obj[name] = field.as<double>();
              break;
            case PgFieldTypeOID::boolean:
              obj[name] = field.as<bool>();
              break;
            case PgFieldTypeOID::json:
            case PgFieldTypeOID::jsonb:
              obj[name] = boost::json::parse(field.c_str());
              break;
            case PgFieldTypeOID::bit:
              if (field.size() == 1) {
                obj[name] = (field.c_str()[0] == '1');
                break;
              }
            default:
              obj[name] = field.c_str();
              break;
            }
        }
    }
    // Assuming the function needs to return a boost::json::value containing the boost::json::object
    return boost::json::value(std::move(obj));
  }

  static boost::json::value result_to_json(pqxx::result const& result) {
    boost::json::array jsonArray;
    for(pqxx::row const& row : result) {
      jsonArray.emplace_back(row_to_json(row));
    }
    return boost::json::value{std::move(jsonArray)};
  }

  boost::json::value QResult::Json() const {
    boost::json::object obj;
    obj["ok"] = this->ok;
    obj["msg"] = this->msg;
    obj["data"] = (this->ok ? result_to_json(this->data) : boost::json::array{});
    return boost::json::value{std::move(obj)};
  }

  boost::json::value QResult::DataAsJson() const {
    if (!this->ok) { return boost::json::value{std::move(boost::json::array{})}; }
    return result_to_json(this->data);
  }

  std::ostream& operator<<(std::ostream& os, pqxx::result const& result) {
    os << result_to_json(result);
    return os;
  }

  LogStream& operator<<(LogStream& stream, pqxx::result const& result) {
    stream << result_to_json(result);
    return stream;
  }

  std::ostream& operator<<(std::ostream& os, QResult const& qResult) {
    os << qResult.Json();
    return os;
  }

  LogStream& operator<<(LogStream& stream, QResult const& qResult) {
    stream << qResult.Json();
    return stream;
  }
}