# List all source files for the STNL library
set(STNL_SOURCES
  # Core
  src/core/utils.cpp
  src/core/logger.cpp
  src/core/stnl_module.cpp
  src/core/config.cpp
  # HTTP
  src/http/server.cpp
  src/http/request.cpp
  src/http/session.cpp
  src/http/middleware.cpp
  # DB
  src/db/db.cpp
  src/db/blueprint.cpp
  src/db/sr_param.cpp
  src/db/sr_blueprint.cpp
  src/db/migration.cpp
  src/db/migrator.cpp
  src/db/column.cpp
  src/db/inserter.cpp
  src/db/connection_pool.cpp
)

# Create the single static library
add_library(stnl STATIC ${STNL_SOURCES})

# Apply all required includes to the stnl library
target_include_directories(stnl
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        src
)

# Set Windows SDK version for Boost/Asio compatibility (Windows 7 target)
target_compile_definitions(stnl
    PUBLIC 
        _WIN32_WINNT=0x0601
)

# Apply all required dependencies to the stnl library
target_link_libraries(stnl
    PUBLIC
        Boost::headers
    PRIVATE
        Boost::json
        Boost::url
        Boost::filesystem
        Boost::system
        Boost::uuid
        libpqxx::pqxx
)

# Ensure the final library name is set for installation/export if necessary
set_target_properties(stnl PROPERTIES EXPORT_NAME stnl)