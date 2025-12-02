
#include "stnl/db/db.hpp"
#include "stnl/db/blueprint.hpp"
#include "stnl/db/connection_pool.hpp"
#include "stnl/db/inserter.hpp"
#include "stnl/core/logger.hpp"
#include "stnl/core/utils.hpp"

#include <boost/asio.hpp>
#include <boost/json.hpp>

#include <pqxx/pqxx>
#include <pqxx/result>
#include <pqxx/field.hxx>

#include <functional>
#include <format>
#include <string>
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

  DB::DB(std::string& connStr, asio::io_context& ioc, size_t poolSize, size_t numThreads) : pool_(connStr, poolSize), ioc_(ioc), workGuard_(asio::make_work_guard(ioc_)) {
    /* there has to be at least one mandatory thread */
    if (numThreads == 0) { numThreads = 1; }
    threadPool_.reserve(numThreads);
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
      if (pConn) {
        pqxx::nontransaction tx(*pConn);
        pqxx::result r = tx.exec(qSQL);
        qResult.data = std::move(r);
        qResult.ok = true;
      }
      else {
        qResult.ok = false;
        qResult.msg = "Failed to get a database connection";
      }
    } catch(const pqxx::sql_error& e) {
      qResult.ok = false;
      qResult.msg = Utils::Trim(std::string(e.what()));
      Logger::Err() << "DB::Exec: ErrorWhat: \n" << e.what();
      Logger::Err() << "DB::Exec: ErrorSQL: " << e.query();
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
      if (pConn) {
        // pConn->prepare(sqlCmdName, sqlCmd);
        pqxx::nontransaction tx(*pConn);
        pqxx::result result = tx.exec(sqlCmd, params);
        qResult.data = result;
        qResult.ok = true;
      }
      else {
        qResult.ok = false;
        qResult.msg = "Failed to get a database connection";
      }
    } catch(const pqxx::sql_error& e) {
      qResult.msg = Utils::Trim(std::string(e.what()));
      Logger::Err() << "DB::ExecSQLCmd: ErrorWhat: \n" << e.what();
      Logger::Err() << "DB::ExecSQLCmd: ErrorSQL: " << e.query();
    }
    if (pConn) { pool_.ReturnConnection(pConn); }
    return qResult;
  }

  std::future<QResult> DB::QExec(std::string_view qSQL, bool silent) {
    return Utils::AsFuture<QResult>(ioc_, [this, qSQL, silent = std::move(silent)]() { return this->Exec(qSQL, silent); });
  }
  
  QResult DB::InsertBatch(std::string const& tableName, std::function<void(BatchInserter& batch)> populateBatchFn) {
    BatchInserter batch{tableName};
    populateBatchFn(batch);
    // debug:start
    // if (!silent) { Logger::Dbg() << "DB::Exec:qSQL: \n" << qSQL; }
    QResult qResult{pqxx::result{}, true, ""};
    pqxx::connection* pConn = nullptr;
    try {
      pConn = pool_.GetConnection();
      if (pConn) {
        pqxx::work tx(*pConn);
        for (auto& [SQLCmd, params] : batch.GetSQLCmdLst()) {
          tx.exec(SQLCmd, params);
        }
        tx.commit();
        qResult.ok = true;
      }
      else {
        qResult.ok = false;
        qResult.msg = "Failed to get a database connection";
      }
    } catch(const pqxx::sql_error& e) {
      qResult.ok = false;
      qResult.msg = Utils::Trim(std::string(e.what()));
      Logger::Err() << "DB::InsertBatch: (ROOLBACK) - ErrorWhat: \n" << e.what();
      Logger::Err() << "DB::InsertBatch: ErrorSQL: " << e.query();
    }
    if (pConn) { pool_.ReturnConnection(pConn); }
    return qResult;
  }

  std::future<QResult> DB::QInsertBatch(std::string const& tableName, std::function<void(BatchInserter& batch)> populateBatchFn) {
    return Utils::AsFuture<QResult>(ioc_, [this, tableName = std::string(tableName),  populateBatchFn = std::move(populateBatchFn)]() {
      return this->InsertBatch(tableName, std::move(populateBatchFn));
    });
  }


  void DB::Work(std::function<void(pqxx::work &tx)> doWorkFn) {
    pqxx::connection* pConn = nullptr;
    try {
      pConn = pool_.GetConnection();
      if (pConn) {
        pqxx::work tx(*pConn);
        doWorkFn(tx);
      }
      else {
        Logger::Err() << "BD::Work: failed to get connection. nullptr returned";
      }
    } catch(std::exception const& e) {
      Logger::Err() << "DB::Work: Error: \n" << e.what();
    }
    if (pConn) { pool_.ReturnConnection(pConn); }
  }

  std::future<void> DB::QWork(std::function<void(pqxx::work &tx)> doWorkFn) {
    return Utils::AsFuture<void>(ioc_, [this, doWorkFn = std::move(doWorkFn)]() {
      this->Work(std::move(doWorkFn));
    });
  }

  Migration& DB::GetMigration() {
    return migration_;
  }


  bool DB::TableExists(std::string_view const tableName) {
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

  std::vector<std::string> DB::GetTableIndexNames(std::string_view tableName) {
    std::string qSQL{"SELECT indexname FROM pg_indexes WHERE LOWER(tablename) = LOWER('" + std::string(tableName) + "')"};
    QResult r = this->Exec(qSQL);
    std::vector<std::string> indexNameLst;
    if (r.ok) {
      indexNameLst.reserve(r.data.size());
      for (const auto& row : r.data) {
        indexNameLst.emplace_back(row[0].as<std::string>());
      }
    }
    return indexNameLst;
  }

  std::vector<Column> DB::GetTableColumns(std::string_view tableName) {

      std::vector<std::string> indexNameLst = this->GetTableIndexNames(tableName);
      SelectedFieldIndex sfi;
      
      std::vector<std::string> select;
      select.reserve(10);
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
      where.reserve(4);
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
          
          Column col(table, colName, SQLDataType::Undefined); 

          std::string standardIndexName = Utils::StringToLower(std::format("{}_{}_idx", table, colName));
          std::string uniqueIndexName = Utils::StringToLower(std::format("{}_{}_key", table, colName));
          for (std::string& indexName: indexNameLst) {
            if (!strcmpi(indexName.c_str(), standardIndexName.c_str())) { col.index = true; }
            else if (!strcmpi(indexName.c_str(), uniqueIndexName.c_str())) { col.unique = true; }
            if (col.index && col.unique) { break; }
          }
          
          // A. Handle Nullability
          col.nullable = (isNullable == "YES");

          // B. Handle Data Types and Attributes
          if (dataType == "bigint") {  col.type = SQLDataType::BigInt; }
          else if (dataType == "integer") { col.type = SQLDataType::Integer; }
          else if (dataType == "smallint") {  col.type = SQLDataType::SmallInt;  }
          else if (dataType == "numeric") {
              col.type = SQLDataType::Numeric;
              col.precision = row[sfi.Index("numeric_precision")].as<unsigned short>(0); 
              col.scale = row[sfi.Index("numeric_scale")].as<unsigned short>(0);     
          }
          else if (dataType == "bit") {
              col.type = SQLDataType::Bit;
              col.length = row[sfi.Index("character_maximum_length")].as<std::size_t>(1);
          }
          else if (dataType == "character") {
              col.type = SQLDataType::Char;
              col.length = row[sfi.Index("character_maximum_length")].as<std::size_t>(0);
          }
          else if (dataType == "character varying") {
              col.type = SQLDataType::Varchar;
              col.length = row[sfi.Index("character_maximum_length")].as<std::size_t>(255);
          }
          else if (dataType == "boolean") { col.type = SQLDataType::Boolean; }
          else if (dataType == "date") { col.type = SQLDataType::Date; }
          else if (dataType.find("timestamp") != std::string::npos) { 
              col.type = SQLDataType::Timestamp;
              col.length = row[sfi.Index("numeric_precision")].as<std::size_t>(6);
          }
          else if (dataType == "uuid") { col.type = SQLDataType::UUID; }
          else if (dataType == "text") { col.type = SQLDataType::Text; }
          else {
              STNL::Logger::Wrn() << ("DB::GetTableColumns: Unsupported data type: " + dataType + " for column " + colName);
              col.type = SQLDataType::Undefined; 
          }

          // C. Handle IDENTITY 
          if (row[sfi.Index("identity_generation")].c_str() && row[sfi.Index("identity_generation")].c_str()[0] != '\0') {
              col.identity = true;
          }
          
          // D. Handle Default Value
          if (row[sfi.Index("column_default")].c_str() && row[sfi.Index("column_default")].c_str()[0] != '\0') {
            std::string s = row[sfi.Index("column_default")].as<std::string>();
            col.defaultValue = s.substr(0, s.find_last_of(':') - 1); // get the default value without the ::type name portion
          }
          columns.push_back(std::move(col));
      }
      return columns;
  }

  Blueprint DB::QueryBlueprint(std::string_view tableName) {
    if (tableName.empty()){
      Logger::Err() << "DB::QueryBlueprint: Table name cannot be empty for Blueprint generation.";
      throw std::invalid_argument("Blueprint generation requires a non-empty table name.");
    }
    std::vector<Column> cols = GetTableColumns(tableName);
    Blueprint bp{std::string(tableName)};
    for (Column &col : cols) {
      bp.AddColumn(std::move(col));
    }
    return bp;
  }

  std::unordered_map<size_t, std::string> const& DB::GetDataTypes() {
    if (dataTypes_.size() < 1) {
      QResult r = this->Exec("SELECT oid, typname FROM pg_type");
      std::unordered_map<size_t, std::string> dataTypes;
      if (r.ok) {
        dataTypes_.reserve(r.data.size());
        for (const auto& row : r.data) {
          dataTypes_[row[0].as<size_t>()] = row[1].as<std::string>();
        }
      }
    }
    return dataTypes_;
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

  
  boost::json::value DB::RowToJson(pqxx::row const& row, std::unordered_map<size_t, std::string> const& dataTypes) {
    boost::json::object obj;
    for (const auto& field : row) {
        const char* fieldName = field.name();
        if (field.is_null()) { 
            obj[fieldName] = nullptr; 
        }
        else {
            auto it = dataTypes.find(field.type());
            if (it != dataTypes.end()) {
              std::string const& typname = it->second;
              if (typname == "int4" || typname == "int8") {
                obj[fieldName] = field.as<long long>();
              }
              else if (typname == "numeric" || typname == "float4" || typname == "float8" || typname == "double") {
                obj[fieldName] = field.as<double>();
              }
              else if (typname == "boolean") {
                obj[fieldName] = field.as<bool>();
              }
              else if (typname == "bit" && field.size() == 1) {
                obj[fieldName] = (field.c_str()[0] == '1');
              }
              else { obj[fieldName] = field.c_str(); }
            }
            else { obj[fieldName] = field.c_str(); }
        }
    }
    // Assuming the function needs to return a boost::json::value containing the boost::json::object
    return obj;
  }

  boost::json::value DB::ConvertPQXXResultToJson(pqxx::result const& result) {
    boost::json::array jsonArray;
    std::unordered_map<size_t, std::string> const& dataTypes = this->GetDataTypes();
    for(pqxx::row const& row : result) {
      jsonArray.emplace_back(DB::RowToJson(row, dataTypes));
    }
    return boost::json::value{std::move(jsonArray)};
  }

  boost::json::value DB::ConvertQResultToJson(QResult const& qResult) {
    boost::json::object obj;
    obj["ok"] = qResult.ok;
    obj["msg"] = qResult.msg;
    obj["data"] = (qResult.ok ? this->ConvertPQXXResultToJson(qResult.data) : boost::json::array{});
    return boost::json::value{std::move(obj)};
  }
}
