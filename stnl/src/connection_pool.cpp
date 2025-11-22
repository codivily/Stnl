
#include "stnl/logger.hpp"
#include "stnl/connection_pool.hpp"
#include <pqxx/pqxx>
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <iostream>

namespace STNL
{

  ConnectionPool::ConnectionPool(std::string &connStr, std::size_t maxSize) : connStr_(connStr), maxSize_(maxSize)
  {
    // Pre-fill the bool with connections
    for (std::size_t i = 0; i < maxSize; ++i)
    {
      try
      {
        auto conn = std::make_unique<pqxx::connection>(connStr_);
        pool_.push_back(std::move(conn));
      }
      catch (const std::exception &e)
      {
        std::cerr << "ConnectionPool::ConnectionPool: Pool initialization failed on connection " << i + 1 << std::endl;
        maxSize_ = pool_.size();
      }
    }
  }

  pqxx::connection *ConnectionPool::GetConnection()
  {
    std::unique_lock<std::mutex> lock(poolMutex_);
    poolCondition_.wait(lock, [this]() { return !pool_.empty(); });
    pqxx::connection *conn = pool_.back().release();
    pool_.pop_back();
    return conn;
  }

  void ConnectionPool::ReturnConnection(pqxx::connection *conn)
  {
    if (!conn) { return; }
    std::unique_lock<std::mutex> lock(poolMutex_);
    pool_.push_back(std::unique_ptr<pqxx::connection>(conn));
    poolCondition_.notify_one();
  }
}