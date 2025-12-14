#ifndef STNL_SERVER_HPP
#define STNL_SERVER_HPP

#include "stnl/http/core.hpp"
#include "stnl/db/db.hpp"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>
#include <boost/json.hpp>

#include <algorithm>
#include <string>
#include <map>
#include <vector>
#include <memory>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace fs = boost::filesystem;
using tcp = boost::asio::ip::tcp;


namespace STNL {

class DB; // forward declaration
class STNLModule; // forward delcaration
class Request; // forward declaration
class Middleware; // forward declaration

struct CharPtrHash {
    std::size_t operator()(volatile const void* key) const noexcept {
        return std::hash<std::uintptr_t>{}(reinterpret_cast<std::uintptr_t>(key));
    }
};

struct CharPtrEqual {
    bool operator()(volatile const void* lhs, volatile const void* rhs) const noexcept {
        return lhs == rhs;
    }
};

class Server {
public:
    Server(asio::io_context& ioc, tcp::endpoint endpoint, fs::path rootDirPath);
    
    void AddDatabase(std::string const& keyAlias, std::string const& connectionString, size_t poolSize = 4, size_t numThreads = 4);
    std::shared_ptr<DB> GetDatabase(std::string const& keyAlias = "default");

    static http::message_generator Response(Request const& req, http::status status_code = http::status::ok);
    static http::message_generator Response(Request const& req, const std::string& msg, http::status status_code = http::status::ok);
    static http::message_generator Response(Request const& req, const fs::path& file_path, std::string content_type, http::status status_code = http::status::ok);
    static http::message_generator Response(Request const& req, const boost::json::value& data, http::status status_code = http::status::ok);

    
    const Router& GetRouter() const;
    void Get(std::string path, RouteHandler handler);
    void Post(std::string path, RouteHandler handler);
    void Put(std::string path, RouteHandler handler);
    void Delete(std::string path, RouteHandler handler);
    void Options(std::string path, RouteHandler handler);
    
    template <typename MiddlewareType>
    void Use()
    {
        static_assert(std::is_base_of_v<Middleware, MiddlewareType>, "MiddlewareType must inherit from Middleware");
        middlewares_.push_back(std::make_unique<MiddlewareType>(*this));
    }
    const std::vector<std::unique_ptr<Middleware>>& GetMiddlewares() const; 

    template <typename ModuleType>
    void AddModule()
    {
        static_assert(std::is_base_of_v<STNLModule, ModuleType>, "ModuleType must inherit from STNLModule");
        static_assert(std::is_same_v<decltype(ModuleType::sType), volatile const char>, "ModuleType must define static char sType");  // Enforce at compile-time
        volatile const void* key = &ModuleType::sType;
        auto it = modules_.find(key);
        if (it == modules_.end()) {
            std::shared_ptr<STNLModule> m = std::static_pointer_cast<STNLModule>(std::make_shared<ModuleType>(*this));
            modules_[key] = m;
            modulesVec_.push_back(m);
        }
    }

    template <typename ModuleType>
    std::shared_ptr<ModuleType> GetModule() {
        static_assert(std::is_base_of_v<STNLModule, ModuleType>, "ModuleType must inherit from STNLModule");
        static_assert(std::is_same_v<decltype(ModuleType::sType), volatile const char>, "ModuleType must define static char sType");  // Enforce at compile-time
        volatile const void* key = &ModuleType::sType;
        auto it = modules_.find(key);
        if (it == modules_.end()) { return nullptr; }
        return std::static_pointer_cast<ModuleType>(it->second);
    }

    fs::path GetRootDirPath();
    void Run();
    asio::io_context& GetIOC();

private:
    void RunDatabaseMigrations();
    void SetupModules();
    void SetupMiddlewares();
    void LaunchModules();
    void DoAccept();
    void OnAccept(beast::error_code ec, tcp::socket socket);
    void AddRoute(http::verb method, std::string path, RouteHandler handler);
    asio::io_context& ioc_;

    std::unordered_map<std::string, std::shared_ptr<DB>> databases_;
    std::vector<std::string> databaseKeyAliases_;

    std::vector<std::shared_ptr<STNLModule>> modulesVec_;
    std::unordered_map<volatile const void*, std::shared_ptr<STNLModule>, CharPtrHash, CharPtrEqual> modules_;
    
    tcp::acceptor acceptor_;  // Fixed: was missing type in original
    Router router_;
    std::vector<std::unique_ptr<Middleware>> middlewares_;
    fs::path rootDirPath_;
};

}

#endif // STNL_SERVER_HPP
