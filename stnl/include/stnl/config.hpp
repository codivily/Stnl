

#include <boost/json.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/beast/core.hpp>

#include <mutex>
#include <string>
#include <stdexcept>


namespace json = boost::json;
namespace fs = boost::filesystem;

namespace STNL {

  class Logger; // forward declaration

  class Config {

    public:
      static void Init(const fs::path& configPath);
      template <typename T>
      static boost::optional<T> Value(const std::string& keyPath, boost::optional<T> defaultValue = boost::none) {
        if (!instance_) { return std::move(defaultValue); }
        return std::move(instance_->GetValue<T>(keyPath, defaultValue));
      }
    private:
      static Config* instance_;
      static std::mutex initMutex_;
      
      json::value data_;

      Config(const fs::path& configPath);
      bool LoadFromFile(const fs::path& configPath);

      template <typename T>
      boost::optional<T> GetValue(const std::string& keyPath, boost::optional<T> defaultValue = boost::none) {
        std::istringstream ss(keyPath);
        std::string token;
        json::value current = data_;
        while (std::getline(ss, token, '.')) {
          if (!current.is_object()) { return defaultValue; }
          json::object obj = current.as_object();
          auto it = obj.find(token);
          if (it == obj.end()) {
            return defaultValue;
          }
          current = it->value();
        }
        try {
          return json::value_to<T>(current);
        } catch (const std::exception& e) {
          Logger::Err("Config::GetValue (\"" + keyPath + "\") conversion error: " + e.what());
        }
        return boost::none;
      }

      // Disallow copy and assignment
      Config(const Config&) = delete;
      Config& operator=(const Config&) = delete;
  };
}