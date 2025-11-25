
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

  ConnectionPool::ConnectionPool(std::string &connStr, std::size_t maxSize) : connStr_(connStr), maxSize_(maxSize), size_(0)
  {
    if (maxSize_ < 1) { maxSize_ = 1; }
    pool_.reserve(maxSize_);
    for (std::size_t i = 0; i < maxSize_; ++i) {
      try {
        pool_.emplace_back(std::make_unique<pqxx::connection>(connStr_));
      }
      catch (const std::exception &e) {
        std::cerr << "ConnectionPool::ConnectionPool: Pool initialization failed on connection " << i + 1 << std::endl;
        maxSize_ = pool_.size();
      }
    }
    size_ = pool_.size();
  }

  pqxx::connection *ConnectionPool::GetConnection()
  {
    std::unique_lock<std::mutex> lock(poolMutex_);
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