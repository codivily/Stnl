#ifndef STNL_CONFIG_HPP
#define STNL_CONFIG_HPP

#include "logger.hpp"

#include <boost/beast/core.hpp>
#include <boost/filesystem.hpp>
#include <boost/json.hpp>
#include <boost/optional.hpp>

#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

namespace json = boost::json;
namespace fs = boost::filesystem;

namespace STNL {

class Config {

  public:
    static auto Instance() -> Config&;

    static auto GetRootDirPath() -> fs::path const&;

    template <typename T>
    static boost::optional<T> Value(const std::string &keyPath, boost::optional<T> defaultValue = boost::none) {
        return  Config::Instance().GetValue<T>(keyPath, defaultValue);
    }

    template <typename T>
    static boost::optional<T> Value(std::string const &keyPath, T defaultValue) {
        return Config::Value<T>(keyPath, boost::optional<T>(defaultValue));
    }

  private:
    static std::unique_ptr<Config> instance_;
    static std::mutex initMutex_;

    json::value data_;
    fs::path rootDirPath_;
    
    Config();
    bool LoadFromFile(const fs::path &configPath);


    template <typename T>
    boost::optional<T> GetValue(const std::string &keyPath, boost::optional<T> defaultValue = boost::none) {
        std::istringstream ss(keyPath);
        std::string token;
        json::value current = data_;
        while (std::getline(ss, token, '.')) {
            if (!current.is_object()) { return defaultValue; }
            json::object obj = current.as_object();
            auto it = obj.find(token);
            if (it == obj.end()) { return defaultValue; }
            current = it->value();
        }
        try {
            return json::value_to<T>(current);
        } catch (const std::exception &e) { Logger::Err() << ("Config::GetValue (\"" + keyPath + "\") conversion error: " + e.what()); }
        return boost::none;
    }

    template <typename T>
    boost::optional<T> GetValue(std::string const &keyPath, T defaultValue) {
        return this->GetValue<T>(keyPath, boost::optional<T>(defaultValue));
    }

    // Disallow copy and assignment
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;
};
} // namespace STNL

#endif // STNL_CONFIG_HPP