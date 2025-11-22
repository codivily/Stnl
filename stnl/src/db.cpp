

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

  QResult DB::Exec(std::string_view qSQL) {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    Logger::Dbg("DB::Exec: Thread ID: " + oss.str() + ", qSQL: " + std::string(qSQL));

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
      qResult.msg = "Error: " + std::string(e.what());
      Logger::Err("DB::Exec: Error: " + std::string(e.what()));
    }
    if (pConn) { pool_.ReturnConnection(pConn); }
    return qResult;
  }

  std::future<QResult> DB::ExecAsync(std::string_view qSQL) {
    using Task = std::packaged_task<QResult()>;
    auto task = std::make_shared<Task>([this, qSQL]() { return this->Exec(qSQL); });
    std::future<QResult> fut = task->get_future();
    asio::post(ioc_, [task]() { (*task)(); });
    return fut;
  }
  
}