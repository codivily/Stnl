


#include "stnl/db/types.hpp"
#include "stnl/db/migrator.hpp"
#include "stnl/db/column.hpp"
#include "stnl/db/sp_blueprint.hpp"
#include "stnl/db/blueprint.hpp"
#include "stnl/db/migration.hpp"
#include "stnl/core/utils.hpp"
#include "stnl/core/logger.hpp"
#include "stnl/db/db.hpp"

#include <functional>
#include <future>
#include <string>
#include <sstream> // For std::stringstream
#include <format>  // Assuming C++20 std::format is available
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <pqxx/pqxx>

#include <iostream>

namespace STNL {

    static std::string GetConstraintName(Column const& col, std::string const& suffix) {
        return std::format("{}_{}_{}",
            Utils::StringToLower(col.tableName),
            Utils::StringToLower(col.name),
            Utils::StringToLower(suffix)
        );
    }

    void Migrator::Migrate(DB& db, Migration const& migration) {
        // Table migration
        std::vector<std::string> const& tableNames = migration.GetTableNames();
        std::unordered_map<std::string, Blueprint> const& blueprints = migration.GetBlueprints();
        for (std::string const& key : tableNames) {
            Blueprint const& bp = blueprints.at(key);
            try {
                ApplyBlueprint(db, bp);
            }
            catch(std::exception const& e) {
                Logger::Err() << "Migrator::Migrate: Error: " + std::string(e.what());
            }
        }

        // Stored Procedure migration
        std::vector<std::string> const& spNames = migration.GetProcedureNames();
        std::unordered_map<std::string, SpBlueprint> const& spBlueprints = migration.GetProcedureBlueprints();
        for (std::string const& key : spNames) {
            SpBlueprint const& spBp = spBlueprints.at(key);
            try {
                ApplyProcedureBlueprint(db, spBp);
            } catch(std::exception const& e) {
                Logger::Err() << "Migrationor::Migrate: Error: " + std::string(e.what());
            }
        }
    }

    void Migrator::Migrate(DB& db) {
        this->Migrate(db, db.GetMigration());
    }

    std::string Migrator::GenerateSQLType(Column const& col) {
        // Note: IDENTITY is handled here as it is part of the type declaration in PostgreSQL
        if (col.type == SQLDataType::Undefined) {
            throw std::invalid_argument("Undefined parameter type");
        }
        switch (col.type) {
            case SQLDataType::BigInt: 
                return col.identity ? "BIGINT GENERATED ALWAYS AS IDENTITY" : "BIGINT";
            case SQLDataType::Integer: 
                return col.identity ? "INTEGER GENERATED ALWAYS AS IDENTITY" : "INTEGER";
            case SQLDataType::SmallInt: 
                return col.identity ? "SMALLINT GENERATED ALWAYS AS IDENTITY" : "SMALLINT";
            case SQLDataType::Numeric: 
                return std::format("NUMERIC({},{})", col.precision, col.scale);
            case SQLDataType::Varchar: 
                return std::format("VARCHAR({})", col.length);
            case SQLDataType::Char:
                return std::format("CHAR({})", col.length);
            case SQLDataType::Text: return "TEXT";
            case SQLDataType::Boolean: return "BOOLEAN";
            case SQLDataType::Date: return "DATE";
            case SQLDataType::Timestamp:
                // Use col.length for fractional second precision (0-6)
                return std::format("TIMESTAMP({}) WITH TIME ZONE", col.precision); 
            case SQLDataType::UUID: return "UUID";
            case SQLDataType::Bit: return std::format("BIT({})", col.length);
            default:
                throw std::invalid_argument("Unsupported column type");
        }
    }

    std::string Migrator::GenerateSpParamSQL(SpParam const &spParam) {
        if (spParam.type == SQLDataType::Undefined) {
            throw std::invalid_argument("Undefined parameter type");
        }
        std::stringstream ss;
        if (spParam.in && spParam.out) { ss << "INOUT"; }
        else if (spParam.out) { ss << "OUT"; }
        else { ss << "IN"; }
        ss << ' ' << spParam.name << ' ';

        switch (spParam.type) {
            case SQLDataType::BigInt:
                ss << "BIGINT";
                break;
            case SQLDataType::Integer:
                ss << "INTEGER";
                break;
            case SQLDataType::SmallInt:
                ss << "SMALLINT";
                break;
            case SQLDataType::Numeric:
                ss << std::format("NUMERIC({}, {})", spParam.precision, spParam.scale);
                break;
            case SQLDataType::Varchar:
                ss << std::format("VARCHAR({})", spParam.length);
                break;
            case SQLDataType::Char:
                ss << std::format("CHAR({})", spParam.length);
                break;
            case SQLDataType::Text:
                ss << "TEXT";
                break;
            case SQLDataType::Boolean:
                ss << "BOOLEAN";
                break;
            case SQLDataType::Date:
                ss << "DATE";
                break;
            case SQLDataType::Timestamp:
                ss << std::format("TIMESTAMP({}) WITH TIME ZONE", spParam.precision);
                break;
            case SQLDataType::UUID:
                ss << "UUID";
                break;
            case SQLDataType::Bit:
                ss << std::format("BIT({})", spParam.length);
                break;
            default:
                throw std::invalid_argument("Unsupported parameter type");
        }

        if (!spParam.defaultValue.empty()) {
            ss << " DEFAULT " << spParam.defaultValue;
        }
        return ss.str();
    }

    std::string Migrator::GenerateSQLConstraints(const Column& col) {
        std::string constraints;
        if (!col.nullable) { constraints += " NOT NULL"; }
        if (!col.defaultValue.empty()) {
            constraints += std::format(" CONSTRAINT {} DEFAULT {}",
                GetConstraintName(col, "default"),
                col.defaultValue
            );
        }
        return constraints;
    }



  std::string Migrator::GenerateCreateSQL(Blueprint const& bp) {
      std::stringstream ss;
      ss << std::format("CREATE TABLE {} (\n", bp.GetTableName());
      std::vector<std::string> const& columnNames = bp.GetColumnNames();
      std::unordered_map<std::string, Column> const& columns = bp.GetColumns();

      size_t i = 0;
      for (std::string const& key : columnNames) {
        Column const& col = columns.at(key);
        ss << std::format("  {} {}{}", col.realName, GenerateSQLType(col), GenerateSQLConstraints(col));
        if (i < columns.size() - 1) {
          ss << ',';
        }
        ss << '\n';
        i++;
      }      
      ss << ");\n";
      i = 0; // reset for reuse
      for (std::string const& key : columnNames) {
        Column const& col = columns.at(key);
        if (col.unique || col.index) {
            if (col.unique) {
                ss << std::format("CREATE UNIQUE INDEX {} ON {} ({});\n",
                    GetConstraintName(col, "key"),
                    col.tableName,
                    col.realName
                );
            }
            else if (col.index) {
                ss << std::format("CREATE INDEX {} ON {} ({});\n",
                    GetConstraintName(col, "idx"),
                    col.tableName,
                    col.realName
                );
            }
        }
        ++i;
      }
      return Utils::FixIndent(ss.str());
    }

    // Helper to check if type and type parameters match (Length, Precision, Identity)
    static bool SQLDataTypeAndParamsMatch(const Column& current, const Column& desired) {
        if (current.type != desired.type) {
            Logger::Dbg() << "type-mismatch: " << current.name;
            return false;
        }
        if (desired.type == SQLDataType::Numeric) {
            if (current.precision != desired.precision || current.scale != desired.scale) {
                Logger::Dbg() << "numberic-precision-mismatch: " << current.name;
                return false;
            }
        }
        else if (desired.type == SQLDataType::Varchar || 
                  desired.type == SQLDataType::Char || 
                  desired.type == SQLDataType::Bit) {
            if (current.length != desired.length) {
                Logger::Dbg() << "(varchar|char|bit)-length-mismatch: " << current.name;
                return false;
            }
        }
        return true;
    }
    
    // --- Core ApplyBlueprint Logic ---

    void Migrator::ApplyBlueprint(DB& db, Blueprint const& bp) {
        // NOTE: Removed previous Dbg/QExec calls to focus on migration logic.
        std::string const& tableName = bp.GetTableName();
        
        // 1. Check for table existence
        bool bTableExists = db.TableExists(tableName);
        
        // If table does not exist, create the full table
        if (!bTableExists) {
            Logger::Inf() << std::format("Migrator: Table '{}' does not exist. Creating.", tableName);
            std::string createSQL = GenerateCreateSQL(bp);
            QResult r = db.Exec(createSQL);
            if (!r.ok) {
                Logger::Err() << std::format("Migrator: Failed to create table {}: {}", tableName, r.msg);
                throw std::runtime_error("Migration failed (CREATE TABLE) on " + tableName);
            }
            return; // Table created, migration complete for this blueprint
        }

        // 2. Table exists, retrieve current schema
        Blueprint oldBp = db.QueryBlueprint(tableName);
        
        // 3. Compare blueprints and generate ALTER SQL
        std::vector<std::string> alterStatements;
        
        std::vector<std::string> const& desiredColumnNames = bp.GetColumnNames();
        auto const& desiredColumns = bp.GetColumns();
        auto const& currentColumns = oldBp.GetColumns();

        // for (const auto& pair : desiredColumns) {
        for (std::string const& desiredNameLower : desiredColumnNames) {
            Column const& desiredCol = desiredColumns.at(desiredNameLower);
            auto it = currentColumns.find(desiredNameLower);
            // A. Column does not exist -> ADD COLUMN
            if (it == currentColumns.end()) {
                std::string addSQL = Utils::FixIndent(std::format(R"(
                    ALTER TABLE {} ADD COLUMN {} {}{};
                )", tableName, desiredCol.realName, GenerateSQLType(desiredCol), GenerateSQLConstraints(desiredCol)));
                alterStatements.emplace_back(addSQL);
                Logger::Inf() << std::format("Migrator: {}", addSQL);
                continue;
            }

            // B. Column exists -> Check for MODIFICATIONS
            Column const& currentCol = it->second;

            // Check 1: Type and Parameters (Length, Precision)
            if (!SQLDataTypeAndParamsMatch(currentCol, desiredCol)) {
                // Generate ALTER COLUMN TYPE statement
                std::string typeSQL = Utils::FixIndent(std::format(R"(
                    ALTER TABLE {} ALTER COLUMN {} TYPE {};
                )", tableName, desiredCol.realName, GenerateSQLType(desiredCol)));

                alterStatements.emplace_back(typeSQL);
                Logger::Inf() << std::format("Migrator: {}", typeSQL);
            }

            // check 2: Identity check
            if (currentCol.identity != desiredCol.identity) {
                std::string identitySQL;
                if (desiredCol.identity) {
                    identitySQL = Utils::FixIndent(std::format(R"(
                        ALTER TABLE {} ALTER COLUMN {} ADD GENERATED BY DEFAULT AS IDENTITY
                    )", tableName, desiredCol.realName));
                }
                else {
                    identitySQL = Utils::FixIndent(std::format(R"(
                        ALTER TABLE {} ALTER COLUMN {} DROP IDENTITY
                    )", tableName, desiredCol.realName));
                }
                alterStatements.emplace_back(identitySQL);
                Logger::Inf() << std::format("Migrator: {}", identitySQL);
            }

            // Check 2: Nullability (Requires separate ALTER commands in PostgreSQL)
            if (currentCol.nullable != desiredCol.nullable) {
                std::string nullSQL;
                if (desiredCol.nullable) {
                    nullSQL = std::format("ALTER TABLE {} ALTER COLUMN {} DROP NOT NULL;", tableName, desiredCol.realName);
                } else {
                    nullSQL = std::format("ALTER TABLE {} ALTER COLUMN {} SET NOT NULL;", tableName, desiredCol.realName);
                }
                alterStatements.emplace_back(nullSQL);
                Logger::Inf() << std::format("Migrator: {}", nullSQL);
            }
            
            // Check 3: Default Value
            
            std::string desiredDefaultValue = std::string(desiredCol.defaultValue);
            if (desiredCol.type == SQLDataType::UUID &&
                desiredCol.identity &&
                desiredCol.defaultValue.empty()) {
                  desiredDefaultValue = std::string("uuidv7()");
            }
            if (strcmp(currentCol.defaultValue.c_str(), desiredDefaultValue.c_str()) != 0) {
                std::string defaultSQL;
                if (desiredDefaultValue.empty()) {
                    // Remove default
                    defaultSQL = std::format("ALTER TABLE {} ALTER COLUMN {} DROP DEFAULT;", tableName, desiredCol.realName);
                } else {
                    // Set or change default
                    defaultSQL = std::format("ALTER TABLE {} ALTER COLUMN {} SET DEFAULT {};", tableName, desiredCol.realName, desiredDefaultValue);
                }
                alterStatements.emplace_back(defaultSQL);
                Logger::Inf() << std::format("Migrator: {}", defaultSQL);
            }

            if (currentCol.unique != desiredCol.unique) {
                std::string uniqueIndexSQL;
                if (desiredCol.unique) {
                    uniqueIndexSQL = std::format("CREATE UNIQUE INDEX CONCURRENTLY {} ON {} ({})",
                        GetConstraintName(desiredCol, "key"),
                        desiredCol.tableName,
                        desiredCol.realName
                    );
                }
                else {
                    uniqueIndexSQL = std::format("DROP INDEX CONCURRENTLY IF EXISTS {}", GetConstraintName(currentCol, "key"));
                }
                alterStatements.emplace_back(uniqueIndexSQL);
            }
            // there should either be unique index or a standard index on a column but not both at the same time for a performance reason
            if (currentCol.index != desiredCol.index || desiredCol.unique) {
                std::string indexSQL;
                if (desiredCol.index && !desiredCol.unique) {
                    indexSQL = std::format("CREATE INDEX CONCURRENTLY IF NOT EXISTS {} ON {} ({})",
                        GetConstraintName(desiredCol, "idx"),
                        desiredCol.tableName,
                        desiredCol.realName
                    );
                }
                else if (currentCol.index) {
                    indexSQL = std::format("DROP INDEX CONCURRENTLY IF EXISTS {}", GetConstraintName(currentCol, "idx"));
                }
                if (!indexSQL.empty()) { alterStatements.emplace_back(indexSQL); }
            }
        }

        // 4. Execute ALTER statements
        if (alterStatements.empty()) {
            Logger::Inf() << std::format("Migrator: Table '{}' is already up to date.", tableName);
            return;
        }

        Logger::Inf() << std::format("Migrator: Applying {} change(s) to table '{}'.", alterStatements.size(), tableName);
        // Execute all statements in order
        for (const std::string& sql : alterStatements) {
            std::string fixedSQL = Utils::FixIndent(sql); 
            QResult r = db.Exec(fixedSQL, true);
            if (!r.ok) {
                Logger::Err() << std::format("Migrator: Failed ALTER SQL: {} Error: {}", fixedSQL, r.msg);
                throw std::runtime_error("Migration failed (ALTER TABLE) due to SQL error on table " + tableName); 
            }
        }
        Logger::Inf() << std::format("Migrator: Change(s) applied to table {}", tableName);

    }

    void Migrator::ApplyProcedureBlueprint(DB& db, SpBlueprint const& spBp) {

        std::stringstream ss;
        ss << std::format("CREATE OR REPLACE PROCEDURE {}", spBp.GetName());
        std::unordered_map<std::string, SpParam> const& params = spBp.GetParams();
        ss << '(';
        if (params.size() > 0) {
            size_t i = 0;
            for(std::string const &paramName : spBp.GetParamNames()) {
                SpParam const& spParam = params.at(paramName);
                if (i++ > 0) { ss << ", "; }
                ss << this->GenerateSpParamSQL(spParam);
            }
        }
        ss << ")\n";
        ss << "LANGUAGE plpgsql\n";
        ss << "AS $$\n";
        ss << "BEGIN\n";
        ss << std::string(spBp.GetBody());
        ss << ";\nEND;\n";
        ss << "$$;\n";
        std::string qSQL = Utils::FixIndent(ss.str());
        QResult r = db.Exec(qSQL);
        if (!r.ok) {
            Logger::Err() << std::format("Migrator: Failed CREATE OR REPLACE PROCEDURE. SQL: {} Error: {}", qSQL, r.msg);
            throw std::runtime_error("Migration failed (CREATE OR REPLACE PROCEDURE) due to SQL error. Procedure name: " + std::string(spBp.GetName()));
        }
        Logger::Inf() << std::format("Migrator: PROCEDURE CREATED/REPLACED: {}", spBp.GetName());
    } 
}
