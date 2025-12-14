
#include "stnl/blueprint.hpp"
#include "stnl/db.hpp"
#include "stnl/logger.hpp"
#include "stnl/migrator.hpp"
#include "stnl/utils.hpp"

#include <boost/asio.hpp>
#include <boost/json.hpp>

#include <chrono>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility> // for std::make_pair()

namespace asio = boost::asio;
namespace json = boost::json;

using Logger = STNL::Logger;
using DB = STNL::DB;
using Migrator = STNL::Migrator;
using Blueprint = STNL::Blueprint;
using QResult = STNL::QResult;

int main(int argc, char argv[]) {
    std::string dbName = "stnl_db";
    std::string dbUser = "postgres";
    std::string dbPassword = "postgres";
    std::string dbHost = "localhost";
    size_t dbPort = 5432;
    std::string dbSchema = "public";
    std::string connStr = DB::GetConnectionString(dbName, dbUser, dbPassword, dbHost, dbPort, dbSchema);

    asio::io_context ioc;
    Logger::Init(ioc);
    DB db(connStr, ioc);
    // STNL::ConnectionPool pool{connStr, 4};
    // auto pCon = std::make_shared<pqxx::connection>("dbname=stnl_db
    // user=postgres password=postgres host=localhost port=5432
    // options=-csearch_path=stnl_sch,public");
    Migrator migrator;

    migrator.Table("Product", [](Blueprint &bp) {
        bp.BigInt("id").Identity();
        bp.UUID("uuid").Default();
        bp.Varchar("name").Length(255).NotNull();
        bp.Numeric("price").Precision(9).Scale(2).Null();
        bp.Text("description").Null();
        bp.Timestamp("utcdt").NotNull().Default();
        bp.Bit("active").N(1).NotNull().Default();
    });
    migrator.Migrate(db);

    // QResult r = db.QExec("SELECT * FROM product ORDER BY utcdt DESC").get();
    // Logger::Err() << r.data;
    Logger::Dbg() << "::main:: before inserts' loop";
    unsigned int maxCount = 15;
    unsigned int i = 0;
    while (true) {
        ++i;
        std::future<QResult> fut = db.QInsert<true>("Product", std::make_pair("name", "Oreshnic - " + std::to_string(i)),
                                                    std::make_pair("price", 10.5 + static_cast<double>(i)), std::make_pair("description", nullptr));
        if (i >= maxCount) {
            Logger::Dbg() << "::main:: waiting for last insert";
            Logger::Dbg() << fut.get(); // wait for the last insert to finish
            break;
        }
    }
    Logger::Dbg() << "::main:: after inserts' loop";
    ioc.run();
    return 0;
}