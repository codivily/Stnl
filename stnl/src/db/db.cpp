
#include "stnl/db/db.hpp"
#include "stnl/core/logger.hpp"
#include "stnl/core/utils.hpp"
#include "stnl/db/blueprint.hpp"
#include "stnl/db/connection_pool.hpp"
#include "stnl/db/inserter.hpp"

#include <boost/asio.hpp>
#include <boost/json.hpp>

#include <pqxx/field.hxx>
#include <pqxx/pqxx>
#include <pqxx/result>

#include <format>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

namespace asio = boost::asio;
namespace json = boost::json;

namespace STNL {

DB::DB(std::string const &connStr, asio::io_context &ioc, size_t poolSize, size_t numThreads)
    : pool_(connStr, poolSize), ioc_(ioc), workGuard_(asio::make_work_guard(ioc_)) {
    /* there has to be at least one mandatory thread */
    if (numThreads == 0) { numThreads = 1; }
    threadPool_.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        threadPool_.emplace_back([this]() { this->ioc_.run(); });
    }
}

DB::~DB() {
    workGuard_.reset();
    for (auto &t : threadPool_) {
        if (t.joinable()) { t.join(); }
    }
}

auto DB::GetIOC() -> asio::io_context & {
    return ioc_;
}

auto DB::Exec(std::string_view qSQL, bool silent) -> QResult {
    if (!silent) { Logger::Dbg() << "DB::Exec:qSQL: \n" << qSQL; }
    QResult qResult{.data = pqxx::result{}, .ok = false, .msg = ""};
    pqxx::connection *pConn = nullptr;
    try {
        pConn = pool_.GetConnection();
        if (pConn != nullptr) {
            pqxx::nontransaction tx(*pConn);
            pqxx::result r = tx.exec(qSQL);
            qResult.data = std::move(r);
            qResult.ok = true;
        } else {
            qResult.ok = false;
            qResult.msg = "Failed to get a database connection";
        }
    } catch (const pqxx::sql_error &e) {
        qResult.ok = false;
        qResult.msg = Utils::Trim(std::string(e.what()));
        Logger::Err() << "DB::Exec: ErrorWhat: \n" << e.what();
        Logger::Err() << "DB::Exec: ErrorSQL: " << e.query();
    }
    if (pConn != nullptr) { pool_.ReturnConnection(pConn); }
    return qResult;
}

auto DB::ExecSQLCmd(std::string const &sqlCmdName, std::string const &sqlCmd, pqxx::params &params, bool silent) -> QResult {
    if (!silent) { Logger::Dbg() << std::format("DB::ExecSQLCmd:<{}>: {}", sqlCmdName, sqlCmd); }
    QResult qResult{.data = pqxx::result{}, .ok = false, .msg = ""};
    pqxx::connection *pConn = nullptr;
    try {
        pConn = pool_.GetConnection();
        if (pConn != nullptr) {
            // pConn->prepare(sqlCmdName, sqlCmd);
            pqxx::nontransaction tx(*pConn);
            pqxx::result result = tx.exec(sqlCmd, params);
            qResult.data = result;
            qResult.ok = true;
        } else {
            qResult.ok = false;
            qResult.msg = "Failed to get a database connection";
        }
    } catch (const pqxx::sql_error &e) {
        qResult.msg = Utils::Trim(std::string(e.what()));
        Logger::Err() << "DB::ExecSQLCmd: ErrorWhat: \n" << e.what();
        Logger::Err() << "DB::ExecSQLCmd: ErrorSQL: " << e.query();
    }
    if (pConn != nullptr) { pool_.ReturnConnection(pConn); }
    return qResult;
}

auto DB::QExec(std::string_view qSQL, bool silent) -> std::future<QResult> {
    return Utils::AsFuture<QResult>(ioc_, [this, qSQL, silent = silent]() { return this->Exec(qSQL, silent); });
}

auto DB::InsertBatch(std::string const &tableName, const std::function<void(BatchInserter &batch)> &populateBatchFn) -> QResult {
    BatchInserter batch{tableName};
    populateBatchFn(batch);
    // debug:start
    // if (!silent) { Logger::Dbg() << "DB::Exec:qSQL: \n" << qSQL; }
    QResult qResult{.data = pqxx::result{}, .ok = true, .msg = ""};
    pqxx::connection *pConn = nullptr;
    try {
        pConn = pool_.GetConnection();
        if (pConn != nullptr) {
            pqxx::work tx(*pConn);
            for (auto &[SQLCmd, params] : batch.GetSQLCmdLst()) { tx.exec(SQLCmd, params); }
            tx.commit();
            qResult.ok = true;
        } else {
            qResult.ok = false;
            qResult.msg = "Failed to get a database connection";
        }
    } catch (const pqxx::sql_error &e) {
        qResult.ok = false;
        qResult.msg = Utils::Trim(std::string(e.what()));
        Logger::Err() << "DB::InsertBatch: (ROOLBACK) - ErrorWhat: \n" << e.what();
        Logger::Err() << "DB::InsertBatch: ErrorSQL: " << e.query();
    }
    if (pConn != nullptr) { pool_.ReturnConnection(pConn); }
    return qResult;
}

auto DB::QInsertBatch(std::string const &tableName, std::function<void(BatchInserter &batch)> populateBatchFn) -> std::future<QResult> {
    return Utils::AsFuture<QResult>(ioc_, [this, tableName = &tableName, populateBatchFn = std::move(populateBatchFn)]() {
        return this->InsertBatch(*tableName, populateBatchFn);
    });
}

void DB::Work(const std::function<void(pqxx::work &tx)> &doWorkFn) {
    pqxx::connection *pConn = nullptr;
    try {
        pConn = pool_.GetConnection();
        if (pConn != nullptr) {
            pqxx::work tx(*pConn);
            doWorkFn(tx);
        } else {
            Logger::Err() << "BD::Work: failed to get connection. nullptr returned";
        }
    } catch (std::exception const &e) { Logger::Err() << "DB::Work: Error: \n" << e.what(); }
    if (pConn != nullptr) { pool_.ReturnConnection(pConn); }
}

auto DB::QWork(std::function<void(pqxx::work &tx)> doWorkFn) -> std::future<void> {
    return Utils::AsFuture<void>(ioc_, [this, doWorkFn = std::move(doWorkFn)]() { this->Work(doWorkFn); });
}

auto DB::GetMigration() -> Migration & {
    return migration_;
}

auto DB::TableExists(std::string_view const tableName) -> bool {
    std::string qSQL = Utils::FixIndent(R"(
        SELECT 1
        FROM information_schema.tables
        WHERE LOWER(table_name) = LOWER(')" +
                                        pqxx::to_string(tableName) + R"(')
        AND table_schema = CURRENT_SCHEMA;
    )");
    QResult r = this->Exec(qSQL);
    if (!r.ok) { throw std::runtime_error("Failed to query table schema for " + std::string(tableName) + ": " + r.msg); }
    return (!r.data.empty());
}

auto DB::GetTableIndexNames(std::string_view tableName) -> std::vector<std::string> {
    std::string qSQL = std::format("SELECT indexname FROM pg_indexes WHERE LOWER(tablename) = LOWER('{}')", tableName);
    QResult r = this->Exec(qSQL);
    std::vector<std::string> indexNameLst;
    if (r.ok) {
        indexNameLst.reserve(r.data.size());
        for (const auto &row : r.data) { indexNameLst.emplace_back(row[0].as<std::string>()); }
    }
    return indexNameLst;
}

// Helper to map information_schema type strings to Column properties.
static void SetColumnTypeFromData(Column &col, const std::string &dt, const pqxx::row &row) {
    if (dt == "bigint") {
        col.type = SQLDataType::BigInt;
    } else if (dt == "integer") {
        col.type = SQLDataType::Integer;
    } else if (dt == "smallint") {
        col.type = SQLDataType::SmallInt;
    } else if (dt == "numeric") {
        col.type = SQLDataType::Numeric;
        col.precision = row["numeric_precision"].as<unsigned short>(0);
        col.scale = row["numeric_scale"].as<unsigned short>(0);
    } else if (dt == "bit") {
        col.type = SQLDataType::Bit;
        col.length = row["character_maximum_length"].as<std::size_t>(1);
    } else if (dt == "character") {
        col.type = SQLDataType::Char;
        col.length = row["character_maximum_length"].as<std::size_t>(0);
    } else if (dt == "character varying") {
        col.type = SQLDataType::Varchar;
        constexpr std::size_t DEFAULT_VARCHAR_LENGTH = 255;
        col.length = row["character_maximum_length"].as<std::size_t>(DEFAULT_VARCHAR_LENGTH);
    } else if (dt == "boolean") {
        col.type = SQLDataType::Boolean;
    } else if (dt == "date") {
        col.type = SQLDataType::Date;
    } else if (dt.find("timestamp") != std::string::npos) {
        col.type = SQLDataType::Timestamp;
        constexpr std::size_t DEFAULT_TIMESTAMP_PRECISION = 6;
        col.length = row["numeric_precision"].as<std::size_t>(DEFAULT_TIMESTAMP_PRECISION);
    } else if (dt == "uuid") {
        col.type = SQLDataType::UUID;
    } else if (dt == "text") {
        col.type = SQLDataType::Text;
    } else {
        STNL::Logger::Wrn() << "DB::GetTableColumns: Unsupported data type: " << dt << " for column " << col.name;
        col.type = SQLDataType::Undefined;
    }
}

auto DB::GetTableColumns(std::string_view tableName) -> std::vector<Column> {
    std::vector<std::string> indexNameLst = this->GetTableIndexNames(tableName);

    constexpr std::size_t SELECT_RESERVE = 10;
    std::vector<std::string> select;
    select.reserve(SELECT_RESERVE);
    select.emplace_back("table_name");
    select.emplace_back("column_name");
    select.emplace_back("data_type");
    select.emplace_back("character_maximum_length");
    select.emplace_back("numeric_precision");
    select.emplace_back("numeric_scale");
    select.emplace_back("is_nullable");
    select.emplace_back("column_default");
    select.emplace_back("identity_generation");

    std::vector<std::string> where;
    where.reserve(4);
    where.emplace_back("table_schema = CURRENT_SCHEMA()");
    if (!tableName.empty()) { where.emplace_back(std::format("LOWER(table_name) = LOWER('{}')", tableName)); }
    std::string qSQL = std::format("SELECT {} FROM information_schema.columns WHERE {} ORDER "
                                   "BY table_name, ordinal_position",
                                   Utils::Join(select, ","), Utils::Join(where, " AND "));
    QResult r = this->Exec(qSQL);
    std::vector<Column> columns;
    columns.reserve(r.data.size());

    if (!r.ok) {
        STNL::Logger::Err() << ("DB::GetTableColumns: Error executing query: " + r.msg);
        throw std::runtime_error("Failed to query table schema: " + r.msg);
    }

    // Iterate over results and map to Column structures
    for (const auto &row : r.data) {
        auto table = row["table_name"].as<std::string>();
        auto colName = row["column_name"].as<std::string>();
        auto dataType = row["data_type"].as<std::string>();
        auto isNullable = row["is_nullable"].as<std::string>();

        Column col(table, colName, SQLDataType::Undefined);
        auto standardIndexName = std::format("{}_{}_idx", table, colName);
        auto uniqueIndexName = std::format("{}_{}_key", table, colName);
        for (const std::string &indexName : indexNameLst) {
            if (Utils::StringCaseCmp(indexName, standardIndexName)) {
                col.index = true;
            } else if (Utils::StringCaseCmp(indexName, uniqueIndexName)) {
                col.unique = true;
            }
            if (col.index && col.unique) { break; }
        }

        // A. Handle Nullability
        col.nullable = (isNullable == "YES");

        // B. Handle Data Types and Attributes
        // Extract type-mapping into a helper to reduce function complexity.
        auto set_col_type = [&](const std::string &dt) {
            // Delegate to a small static helper implemented below.
            // It will set type and any size/precision/scale fields on the
            // Column object based on the information_schema row.
            SetColumnTypeFromData(col, dt, row);
        };
        set_col_type(dataType);

        // C. Handle IDENTITY
        auto identity_generation = row["identity_generation"].as<std::string>("");
        if (!identity_generation.empty() && identity_generation[0] != '\0') { col.identity = true; }

        // D. Handle Default Value
        auto column_default = row["column_default"].as<std::string>("");
        if (!column_default.empty() && column_default[0] != '\0') {
            const auto &s = column_default;
            auto pos = s.find_last_of(':');
            if (pos != std::string::npos && pos > 0) {
                col.defaultValue = s.substr(0, pos - 1);
            } else {
                col.defaultValue = s;
            }
        }
        columns.push_back(std::move(col));
    }
    return columns;
}

auto DB::QueryBlueprint(std::string_view tableName) -> Blueprint {
    if (tableName.empty()) {
        Logger::Err() << "DB::QueryBlueprint: Table name cannot be empty for "
                         "Blueprint generation.";
        throw std::invalid_argument("Blueprint generation requires a non-empty table name.");
    }
    std::vector<Column> cols = GetTableColumns(tableName);
    Blueprint bp{std::string(tableName)};
    for (Column &col : cols) { bp.AddColumn(std::move(col)); }
    return bp;
}

auto DB::GetDataTypes() -> std::unordered_map<size_t, std::string> const & {
    if (dataTypes_.empty()) {
        QResult r = this->Exec("SELECT oid, typname FROM pg_type");
        std::unordered_map<size_t, std::string> dataTypes;
        if (r.ok) {
            dataTypes_.reserve(r.data.size());
            for (const auto &row : r.data) { dataTypes_[row[0].as<size_t>()] = row[1].as<std::string>(); }
        }
    }
    return dataTypes_;
}

auto DB::GetConnectionString(std::string_view dbName, std::string_view dbUser, std::string_view dbPassword, std::string_view dbHost, size_t dbPort,
                             std::string_view dbSchema) -> std::string {
    return std::format("dbname={} user={} password='{}' host={} port={} "
                       "options=-csearch_path={}",
                       dbName, dbUser, dbPassword, dbHost, dbPort, dbSchema);
}

auto DB::RowToJson(pqxx::row const &row, std::unordered_map<size_t, std::string> const &dataTypes) -> boost::json::value {
    boost::json::object obj;
    for (const auto &field : row) {
        const char *fieldName = field.name();
        if (field.is_null()) {
            obj[fieldName] = nullptr;
        } else {
            auto it = dataTypes.find(field.type());
            if (it != dataTypes.end()) {
                std::string const &typname = it->second;
                if (typname == "int4" || typname == "int8") {
                    obj[fieldName] = field.as<long long>();
                } else if (typname == "numeric" || typname == "float4" || typname == "float8" || typname == "double") {
                    obj[fieldName] = field.as<double>();
                } else if (typname == "boolean") {
                    obj[fieldName] = field.as<bool>();
                } else if (typname == "bit" && field.size() == 1) {
                    auto fv = field.as<std::string>();
                    obj[fieldName] = (fv.size() == 1 && fv[0] == '1');
                } else {
                    obj[fieldName] = field.c_str();
                }
            } else {
                obj[fieldName] = field.c_str();
            }
        }
    }
    // Assuming the function needs to return a boost::json::value containing the
    // boost::json::object
    return obj;
}

auto DB::ConvertPQXXResultToJson(pqxx::result const &result) -> boost::json::value {
    boost::json::array jsonArray;
    jsonArray.reserve(result.size());
    std::unordered_map<size_t, std::string> const &dataTypes = this->GetDataTypes();
    for (pqxx::row const &row : result) { jsonArray.emplace_back(DB::RowToJson(row, dataTypes)); }
    return boost::json::value{std::move(jsonArray)};
}

auto DB::ConvertQResultToJson(QResult const &qResult) -> boost::json::value {
    boost::json::object obj;
    obj["ok"] = qResult.ok;
    obj["msg"] = qResult.msg;
    obj["data"] = (qResult.ok ? this->ConvertPQXXResultToJson(qResult.data) : boost::json::array{});
    return boost::json::value{std::move(obj)};
}
} // namespace STNL
