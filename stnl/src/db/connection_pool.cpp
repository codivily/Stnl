
#include "stnl/db/connection_pool.hpp"
#include "stnl/core/logger.hpp"

#include <algorithm>
#include <pqxx/pqxx>

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace STNL {

ConnectionPool::ConnectionPool(std::string connStr, std::size_t maxSize) : connStr_(std::move(connStr)), maxSize_(maxSize), size_(0) {
    maxSize_ = std::max<size_t>(maxSize_, 1);
    pool_.reserve(maxSize_);
}

auto ConnectionPool::GetConnection() -> pqxx::connection * {
    std::unique_lock<std::mutex> lock(poolMutex_);
    if (size_ < 1 && pool_.size() < maxSize_) {
        try {
            pool_.emplace_back(std::make_unique<pqxx::connection>(connStr_));
            return pool_.back().release();
        } catch (const std::exception &e) {
            Logger::Err() << "ConnectionPool::GetConnection: Error: " << std::string(e.what());
            return nullptr;
        }
    }
    poolCondition_.wait(lock, [this]() { return size_ > 0; });
    return pool_[--size_].release();
}

void ConnectionPool::ReturnConnection(pqxx::connection *pConn) {
    if (pConn == nullptr) { return; }
    std::unique_lock<std::mutex> lock(poolMutex_);
    pool_[size_++].reset(pConn);
    poolCondition_.notify_one();
}
} // namespace STNL
