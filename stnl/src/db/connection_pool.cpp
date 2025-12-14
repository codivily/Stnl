
#include "stnl/core/logger.hpp"
#include "stnl/db/connection_pool.hpp"

#include <pqxx/pqxx>

#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <iostream>

namespace STNL
{

  ConnectionPool::ConnectionPool(std::string const& connStr, std::size_t maxSize) : connStr_(connStr), maxSize_(maxSize), size_(0)
  {
    if (maxSize_ < 1) { maxSize_ = 1; }
    pool_.reserve(maxSize_);
  }

  pqxx::connection *ConnectionPool::GetConnection()
  {
    std::unique_lock<std::mutex> lock(poolMutex_);
    if (size_ < 1 && pool_.size() < maxSize_) {
      try {
        pool_.emplace_back(std::make_unique<pqxx::connection>(connStr_));
        return pool_.back().release();
      }
      catch(const std::exception &e) {
        Logger::Err() << "ConnectionPool::GetConnection: Error: " << std::string(e.what());
        return nullptr;
      }
    }
    poolCondition_.wait(lock, [this]() { return size_ > 0; });
    return pool_[--size_].release();
  }

  void ConnectionPool::ReturnConnection(pqxx::connection *pConn)
  {
    if (!pConn) { return; }
    std::unique_lock<std::mutex> lock(poolMutex_);
    pool_[size_++].reset(pConn);
    poolCondition_.notify_one();
  }
}
