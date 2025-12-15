#include "stnl/core/config.hpp"

#include "stnl/core/logger.hpp"

#include <boost/dll.hpp>
#include <boost/filesystem.hpp>
#include <boost/json.hpp>
#include <boost/optional.hpp>
#include <boost/system.hpp>

#include <fstream>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <memory>

namespace json = boost::json;
namespace fs = boost::filesystem;

namespace STNL {

std::unique_ptr<Config> Config::instance_ = nullptr;
std::mutex Config::initMutex_;

auto Config::GetRootDirPath() -> fs::path const& {
    return Config::Instance().rootDirPath_;
}

auto Config::Instance() -> Config& {
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(initMutex_);
        if (instance_ == nullptr) {
            instance_.reset(new Config());
        }
    }
    return *instance_;
}

Config::Config(): rootDirPath_(boost::dll::program_location().parent_path()) {
    bool bLoadError = false;
    fs::path configPath;
    if (fs::exists(rootDirPath_ / "config.local.json")) {
        configPath = rootDirPath_ / "config.local.json";
        bLoadError = !LoadFromFile(configPath);
    }
    else if (fs::exists(rootDirPath_ / "config.json")) {
        configPath = rootDirPath_ / "config.json";
        bLoadError = !LoadFromFile(configPath);
    }
    else {
        Logger::Wrn() << "Config::Config: No config.json or config.local.json file to load";
    }
    if (bLoadError) {
        throw std::runtime_error("Config::Config Failed to load config from: " + configPath.string());
    }
    else if (configPath.c_str()) { Logger::Dbg() << "Config::Config: loaded: " + configPath.string(); }
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
        std::cerr << "Failed to parse config file: " + ec.what() << '\n';
        return false;
    }
    return true;
}
} // namespace STNL
