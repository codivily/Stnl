

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
#include <thread>
#include <memory>
#include <future>

namespace asio = boost::asio;
namespace json = boost::json;

namespace STNL {

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
      // UPDATED ENUM: Added TABLE_NAME at index 0, shifted all others
      enum ColumnIndex
      {
        TABLE_NAME = 0,
        COLUMN_NAME = 1,
        DATA_TYPE = 2,
        CHAR_MAX_LENGTH = 3,
        NUMERIC_PRECISION = 4,
        NUMERIC_SCALE = 5,
        IS_NULLABLE = 6,
        COLUMN_DEFAULT = 7,
        IDENTITY_GENERATION = 8
      };
      // Dynamic WHERE clause construction
      std::string whereClause = "";
      if (!tableName.empty()) {
          // If a table name is provided, filter by it (case-insensitive)
          whereClause = R"(
          WHERE
              LOWER(table_name) = LOWER(')" + pqxx::to_string(tableName) + R"(')
              AND table_schema = CURRENT_SCHEMA()
          )";
      } else {
          // If no table name is provided, query all tables in the current schema
          whereClause = R"(
          WHERE
              table_schema = CURRENT_SCHEMA()
          )";
      }

      std::string qSQL = STNL::Utils::FixIndent(R"(
          SELECT
              table_name, 
              column_name,
              data_type,
              character_maximum_length,
              numeric_precision,
              numeric_scale,
              is_nullable,
              column_default,
              identity_generation
          FROM
              information_schema.columns
      )" + whereClause + R"(
          ORDER BY
              table_name,
              ordinal_position;
      )");
      
      QResult r = this->Exec(qSQL);
      std::vector<Column> columns;
      
      if (!r.ok) {
          STNL::Logger::Err() << ("DB::GetTableColumns: Error executing query: " + r.msg);
          throw std::runtime_error("Failed to query table schema: " + r.msg);
      }

      // Iterate over results and map to Column structures
      for (const auto& row : r.data) {
          std::string table = row[ColumnIndex::TABLE_NAME].as<std::string>();
          std::string colName = row[ColumnIndex::COLUMN_NAME].as<std::string>();
          std::string dataType = row[ColumnIndex::DATA_TYPE].as<std::string>();
          std::string isNullable = row[ColumnIndex::IS_NULLABLE].as<std::string>();
          
          Column col(table, colName, ColumnType::Undefined); 

          // A. Handle Nullability
          col.nullable = (isNullable == "YES");

          // B. Handle Data Types and Attributes
          if (dataType == "bigint") {  col.type = ColumnType::BigInt; }
          else if (dataType == "integer") { col.type = ColumnType::Integer; }
          else if (dataType == "smallint") {  col.type = ColumnType::SmallInt;  }
          else if (dataType == "numeric") {
              col.type = ColumnType::Numeric;
              col.precision = row[ColumnIndex::NUMERIC_PRECISION].as<unsigned short>(0); 
              col.scale = row[ColumnIndex::NUMERIC_SCALE].as<unsigned short>(0);     
          }
          else if (dataType == "bit") {
              col.type = ColumnType::Bit;
              col.length = row[ColumnIndex::CHAR_MAX_LENGTH].as<std::size_t>(1);
          }
          else if (dataType == "character") {
              col.type = ColumnType::Char;
              col.length = row[ColumnIndex::CHAR_MAX_LENGTH].as<std::size_t>(0);
          }
          else if (dataType == "character varying") {
              col.type = ColumnType::Varchar;
              col.length = row[ColumnIndex::CHAR_MAX_LENGTH].as<std::size_t>(255);
          }
          else if (dataType == "boolean") { col.type = ColumnType::Boolean; }
          else if (dataType == "date") { col.type = ColumnType::Date; }
          else if (dataType.find("timestamp") != std::string::npos) { 
              col.type = ColumnType::Timestamp;
              col.length = row[ColumnIndex::NUMERIC_PRECISION].as<std::size_t>(6);
          }
          else if (dataType == "uuid") { col.type = ColumnType::UUID; }
          else if (dataType == "text") { col.type = ColumnType::Text; }
          else {
              STNL::Logger::Wrn() << ("DB::GetTableColumns: Unsupported data type: " + dataType + " for column " + colName);
              col.type = ColumnType::Undefined; 
          }

          // C. Handle IDENTITY 
          if (row[ColumnIndex::IDENTITY_GENERATION].c_str() && row[ColumnIndex::IDENTITY_GENERATION].c_str()[0] != '\0') {
              col.identity = true;
          }
          
          // D. Handle Default Value
          if (row[ColumnIndex::COLUMN_DEFAULT].c_str() && row[ColumnIndex::COLUMN_DEFAULT].c_str()[0] != '\0') {
              col.defaultValue = row[ColumnIndex::COLUMN_DEFAULT].as<std::string>();
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
            Logger::Dbg() << pqxx::to_string(field.type());
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

  boost::json::value QResult::json() const {
    boost::json::object obj;
    obj["ok"] = this->ok;
    obj["msg"] = this->msg;
    obj["data"] = (this->ok ? result_to_json(this->data) : boost::json::array{});
    return boost::json::value{std::move(obj)};
  }
}