
#include "stnl/inserter.hpp"
#include "stnl/logger.hpp"

#include <pqxx/pqxx>

#include <vector>
#include <string>
#include <utility>
#include <sstream>
#include <tuple>

namespace STNL {
 
  std::tuple<std::string, pqxx::params> Inserter::flush(std::string const& tableName) {
    std::string sqlCmd = std::format("INSERT INTO {} ({}) VALUES ({})", tableName, columnsSS_.str(), placeholderSS_.str());
    isFirstPair_ = true;
    columnsSS_.str("");
    columnsSS_.clear();
    placeholderSS_.str("");
    placeholderSS_.clear();
    pqxx::params params{params_};
    params_ = pqxx::params{};
    return {std::move(sqlCmd), std::move(params)};
  }
  
  BatchInserter::BatchInserter(std::string const& tableName) : tableName_(tableName) {}
  
  void BatchInserter::SetTableName(std::string const& tableName) {
    tableName_ = std::string{tableName};
  }

  void BatchInserter::flush() {
    auto [SQLCmd, params] = inserter_.flush(tableName_);
    // Logger::Dbg() << "BatchInserter::flush(): SQLCmd: " << SQLCmd << " | params.size():" << std::to_string(params.size());
    this->SQLCmdLst_.emplace_back(std::move(SQLCmd), std::move(params));
  }

  std::vector<std::pair<std::string, pqxx::params>>& BatchInserter::GetSQLCmdLst() {
    return this->SQLCmdLst_;
  }

}
