

#include "stnl/db.hpp"
#include "stnl/connection_pool.hpp"
#include "stnl/logger.hpp"
#include "stnl/utils.hpp"

#include <boost/asio.hpp>
#include <pqxx/pqxx>

#include <string>
#include <thread>
#include <memory>
#include <future>

namespace asio = boost::asio;

namespace STNL {

  DB::DB(std::string& connStr, asio::io_context& ioc, size_t poolSize, size_t numThreads) : pool_(connStr, poolSize), ioc_(ioc), workGuard_(asio::make_work_guard(ioc_)) {
    /* there has to be at lease one mandatory thread */
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

  QResult DB::Exec(std::string_view qSQL, bool silent) {
    if (!silent) { Logger::Dbg() << "DB::Exec:qSQL: \n" << qSQL; }

    pqxx::connection* pConn = pool_.GetConnection();
    QResult qResult{pqxx::result{}, false, ""};
    try {
      pqxx::connection* pConn = pool_.GetConnection();
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
    using Task = std::packaged_task<QResult()>;
    auto task = std::make_shared<Task>([this, qSQL, silent = std::move(silent)]() { return this->Exec(qSQL, silent); });
    std::future<QResult> fut = task->get_future();
    asio::post(ioc_, [task]() { (*task)(); });
    return fut;
  }

  bool DB::TableExists(std::string_view tableName)
  {
    std::string sql = Utils::FixIndent(R"(
        SELECT 1
        FROM information_schema.tables
        WHERE LOWER(table_name) = LOWER(')" + pqxx::to_string(tableName) + R"(')
        AND table_schema = CURRENT_SCHEMA;
    )");
    Logger::Dbg() << "DB::TableExists: Checking existence of table " << tableName;
    QResult r = this->Exec(sql, false);
    if (!r.ok) { return false; }
    return (r.data.size() < 1 ? false : true);
  }

  std::string DB::GetTypeSQL(Column& column) {
    return "";
  }
}