// Test multi-line logging functionality
#include "stnl/core/logger.hpp"
#include <boost/asio.hpp>
#include <iostream>

int main() {
    boost::asio::io_context ioc;
    STNL::Logger::Init(ioc);
    
    std::cout << "=== Testing Logger Multi-line Support ===" << std::endl << std::endl;
    
    // Test 1: Single-line message
    std::cout << "Test 1: Single-line message" << std::endl;
    STNL::Logger::Inf() << "Server started successfully on port 3777";
    std::cout << std::endl;
    
    // Test 2: Multi-line SQL query
    std::cout << "Test 2: Multi-line SQL query" << std::endl;
    std::string sqlQuery = R"(SELECT 
    users.id,
    users.name,
    users.email
FROM users
WHERE users.active = true
ORDER BY users.created_at DESC)";
    STNL::Logger::Dbg() << "Executing SQL query:\n" << sqlQuery;
    std::cout << std::endl;
    
    // Test 3: Multi-line JSON
    std::cout << "Test 3: Multi-line JSON configuration" << std::endl;
    std::string jsonData = R"({
  "server": "postgres",
  "host": "localhost",
  "port": 5432,
  "database": "myapp"
})";
    STNL::Logger::Inf() << "Database configuration:\n" << jsonData;
    std::cout << std::endl;
    
    // Test 4: Error with stack trace
    std::cout << "Test 4: Error with stack trace" << std::endl;
    STNL::Logger::Err() << "Connection failed:\n" 
                        << "  at DB::Connect() [db.cpp:123]\n"
                        << "  at Server::Init() [server.cpp:45]\n"
                        << "  at main() [main.cpp:10]";
    std::cout << std::endl;
    
    // Test 5: Warning message
    std::cout << "Test 5: Warning message" << std::endl;
    STNL::Logger::Wrn() << "Connection pool size is low: 2/10 connections available";
    std::cout << std::endl;
    
    std::cout << "=== All tests completed! ===" << std::endl;
    return 0;
}
