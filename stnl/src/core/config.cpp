#include "stnl/core/config.hpp"


#include "stnl/core/logger.hpp"

#include <boost/json.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/system.hpp>

#include <mutex>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib> // for std::atexit


namespace json = boost::json;
namespace fs = boost::filesystem;

namespace STNL {

  Config* Config::instance_ = nullptr;
  std::mutex Config::initMutex_;

  void Config::Init(const fs::path& configPath) {
    std::lock_guard<std::mutex> lock(initMutex_);
    if(instance_) { return; }
    instance_ = new Config(configPath);
    std::atexit([]() {
      std::lock_guard<std::mutex> lock(Config::initMutex_);
      if (Config::instance_) {
        delete Config::instance_;
        Config::instance_ = nullptr;
      }
    });
  }
  
  Config::Config(const fs::path& configPath) {
    if (!LoadFromFile(configPath)) {
      throw std::runtime_error("Config::Config Failed to load config from: " + configPath.string());
    }
  }

  bool Config::LoadFromFile(const fs::path& configPath) {
    if (fs::exists(configPath) == false || fs::is_regular_file(configPath) == false) {
      std::cerr << "Config file does not exist or is not a regular file: " + configPath.string() << std::endl;
      return false;
    }
    std::ifstream configFile(configPath.string());
    if (!configFile.is_open()) {
      std::cerr << "Could not open config file: " + configPath.string() << std::endl;
      return false;
    }
    std::string content((std::istreambuf_iterator<char>(configFile)), std::istreambuf_iterator<char>());
    
    boost::system::error_code ec;
    data_ = json::parse(content, ec);
    if (ec) {
      std::cerr << "Failed to parse config file: " + ec.message() << std::endl;
      return false;
    }
    return true;
  }
}
