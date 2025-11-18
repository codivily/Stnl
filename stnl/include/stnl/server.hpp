#ifndef STNL_SERVER_HPP
#define STNL_SERVER_HPP

#include "core.hpp"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <vector>
#include <algorithm>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace fs = boost::filesystem;
using tcp = boost::asio::ip::tcp;


namespace STNL {
    
class STNLModule; // forward delcaration
class Request; // forward declaration

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

class Server : public std::enable_shared_from_this<Server> {
public:
    Server(asio::io_context& ioc, tcp::endpoint endpoint, fs::path rootDirPath);
    
    static http::message_generator Response(const Request& req, http::status status_code, std::string msg);
    static http::message_generator Response(const Request& req, fs::path file_path, std::string content_type);

    void UseMiddleware(Middleware middleware);
    const std::vector<Middleware>& GetMiddlewares() const; 
    const Router& GetRouter() const;
    void Get(std::string path, RouteHandler handler);
    void Post(std::string path, RouteHandler handler);
    void Put(std::string path, RouteHandler handler);
    void Delete(std::string path, RouteHandler handler);
    void Options(std::string path, RouteHandler handler);
    
    template <typename ModuleType>
    void AddModule()
    {
        static_assert(std::is_base_of_v<STNLModule, ModuleType>, "ModuleType must inherit from STNLModule");
        static_assert(std::is_same_v<decltype(ModuleType::sType), volatile const char>, "ModuleType must define static char sType");  // Enforce at compile-time
        volatile const void* key = &ModuleType::sType;
        auto it = modules_.find(key);
        if (it == modules_.end()) {
            std::shared_ptr<STNLModule> m = std::static_pointer_cast<STNLModule>(std::make_shared<ModuleType>(shared_from_this()));
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
    void SetupModules();
    void LaunchModules();
    void DoAccept();
    void OnAccept(beast::error_code ec, tcp::socket socket);
    void AddRoute(http::verb method, std::string path, RouteHandler handler);

    std::vector<std::shared_ptr<STNLModule>> modulesVec_;
    std::unordered_map<volatile const void*, std::shared_ptr<STNLModule>, CharPtrHash, CharPtrEqual> modules_;
    asio::io_context& ioc_;
    tcp::acceptor acceptor_;  // Fixed: was missing type in original
    Router router_;
    std::vector<Middleware> middlewares_;
    fs::path rootDirPath_;
};

}

#endif // STNL_SERVER_HPP