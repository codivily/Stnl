#include "stnl/core/config.hpp"

#include "stnl/core/logger.hpp"

#include <boost/filesystem.hpp>
#include <boost/json.hpp>
#include <boost/optional.hpp>
#include <boost/system.hpp>

#include <cstdlib> // for std::atexit
#include <fstream>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

namespace json = boost::json;
namespace fs = boost::filesystem;

namespace STNL {

std::unique_ptr<Config> Config::instance_ = nullptr;
std::mutex Config::initMutex_;

void Config::Init(const fs::path &configPath) {
    std::lock_guard<std::mutex> lock(initMutex_);
    if (instance_ != nullptr) { return; }
    instance_ = MakeInstance(configPath);
}

Config::Config(const fs::path &configPath) {
    if (!LoadFromFile(configPath)) { throw std::runtime_error("Config::Config Failed to load config from: " + configPath.string()); }
}

auto Config::MakeInstance(const fs::path &configPath) -> std::unique_ptr<Config> {
    return std::unique_ptr<Config>(new Config(configPath));
}

auto Config::LoadFromFile(const fs::path &configPath) -> bool {
    if (!fs::exists(configPath) || !fs::is_regular_file(configPath)) {
        std::cerr << "Config file does not exist or is not a regular file: " + configPath.string() << '\n';
        return false;
    }
    std::ifstream configFile(configPath.string());
    if (!configFile.is_open()) {
        std::cerr << "Could not open config file: " + configPath.string() << '\n';
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(configFile)), std::istreambuf_iterator<char>());

    boost::system::error_code ec;
    data_ = json::parse(content, ec);
    if (ec) {
        std::cerr << "Failed to parse config file: " + ec.message() << '\n';
        return false;
    }
    return true;
}
} // namespace STNL
