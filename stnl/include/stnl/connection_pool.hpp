
#ifndef STNL_CONNECTION_POOL
#define STNL_CONNECTION_POOL

#include <pqxx/pqxx>

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace STNL {
  class ConnectionPool {
    public:
      ConnectionPool(std::string& connStr, size_t maxSize);
      pqxx::connection* GetConnection();
      void ReturnConnection(pqxx::connection *con);

    private:
      std::string connStr_;
      std::vector<std::unique_ptr<pqxx::connection>> pool_;
      std::mutex poolMutex_;
      std::condition_variable poolCondition_;
      size_t maxSize_;
  };
}

#endif // STNL_CONNECTION_POOL