
#ifndef STNL_CONNECTION_POOL
#define STNL_CONNECTION_POOL

#include <pqxx/pqxx>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace STNL {
class ConnectionPool {
  public:
    ConnectionPool(std::string connStr, size_t maxSize);
    pqxx::connection *GetConnection();
    void ReturnConnection(pqxx::connection *pConn);

  private:
    std::string connStr_;
    std::vector<std::unique_ptr<pqxx::connection>> pool_;
    std::mutex poolMutex_;
    std::condition_variable poolCondition_;
    size_t maxSize_;
    size_t size_;
};
} // namespace STNL

#endif // STNL_CONNECTION_POOL
