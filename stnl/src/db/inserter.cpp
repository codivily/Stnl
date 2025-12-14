
#include "stnl/db/inserter.hpp"
#include "stnl/core/logger.hpp"

#include <pqxx/pqxx>

#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace STNL {

auto Inserter::Empty() const -> bool {
    return isFirstPair_;
}

auto Inserter::flush(std::string const &tableName) -> std::tuple<std::string, pqxx::params> {
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

BatchInserter::BatchInserter(std::string tableName) : tableName_(std::move(tableName)) {}

void BatchInserter::SetTableName(std::string const &tableName) {
    tableName_ = std::string{tableName};
}

void BatchInserter::flush() {
    if (inserter_.Empty()) { return; }
    auto [SQLCmd, params] = inserter_.flush(tableName_);
    // Logger::Dbg() << "BatchInserter::flush(): SQLCmd: " << SQLCmd << " |
    // params.size():" << std::to_string(params.size());
    this->SQLCmdLst_.emplace_back(std::move(SQLCmd), std::move(params));
}

auto BatchInserter::GetSQLCmdLst() -> std::vector<std::pair<std::string, pqxx::params>> & {
    if (!inserter_.Empty()) { this->flush(); }
    return this->SQLCmdLst_;
}

} // namespace STNL
