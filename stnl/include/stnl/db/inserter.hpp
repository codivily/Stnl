#ifndef STNL_INSERTER_HPP
#define STNL_INSERTER_HPP


#include <pqxx/pqxx>

#include <vector>
#include <string>
#include <utility>
#include <sstream>


namespace STNL {
  class Inserter {
      public:
        Inserter() = default;
        bool Empty();
        
        std::tuple<std::string, pqxx::params> flush(std::string const& tableName);

        template <typename K, typename T>
        Inserter& operator<<(const std::pair<K, T>& columnValuePair) {
          std::string key{columnValuePair.first};
          this->ProcessPair(key, columnValuePair.second);
          return *this;
        }
        
      private:
        bool isFirstPair_ = true;
        std::stringstream columnsSS_;
        std::stringstream placeholderSS_;
        pqxx::params params_;
        
        template<typename T>
        void ProcessPair(std::string const& key, T value) {
          if (!isFirstPair_) {
            columnsSS_ << ',';
            placeholderSS_ << ',';
          }
          else { isFirstPair_ = false; }
          params_.append(value);
          columnsSS_ << key;
          placeholderSS_ << "$" + std::to_string(params_.size());
        }
    };
  
    
    class BatchInserter {
      public:
        BatchInserter(std::string const& tableName);
        void flush();
        template <typename K, typename T>
        Inserter& operator<<(const std::pair<K, T> columnValuePair) {
          inserter_ << columnValuePair;
          return inserter_;
        }
        void SetTableName(std::string const& tableName);
        std::vector<std::pair<std::string, pqxx::params>>& GetSQLCmdLst();
      private:
        std::string tableName_;
        std::vector<std::pair<std::string, pqxx::params>> SQLCmdLst_;
        Inserter inserter_;
    };
}


#endif // STNL_INSERTER_HPP
