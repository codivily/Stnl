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

if(ENABLE_PCH)
    target_precompile_headers(stnl PUBLIC
        <vector>
        <string>
        <memory>
        <variant>
        <optional>
        <boost/json.hpp>
        <boost/url.hpp>
        <pqxx/pqxx>
    )
endif()

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
set(STNL_PUBLIC_LIBS
    Boost::headers
)

set(STNL_PRIVATE_LIBS
    Boost::json
    Boost::url
    Boost::filesystem
    Boost::system
    libpqxx::pqxx
)

# Conditionally link Boost::uuid if the imported target is available
if(TARGET Boost::uuid)
    list(APPEND STNL_PRIVATE_LIBS Boost::uuid)
elseif(DEFINED Boost_UUID_LIBRARY AND Boost_UUID_LIBRARY)
    message(STATUS "Linking against system Boost UUID library: ${Boost_UUID_LIBRARY}")
    list(APPEND STNL_PRIVATE_LIBS ${Boost_UUID_LIBRARY})
else()
    # Try a direct library lookup for common system library names (libboost_uuid)
    find_library(SYSTEM_BOOST_UUID NAMES boost_uuid libboost_uuid HINTS ${CMAKE_LIBRARY_PATH} PATH_SUFFIXES lib)
    if(SYSTEM_BOOST_UUID)
        message(STATUS "Found system Boost UUID library: ${SYSTEM_BOOST_UUID}; linking against it.")
        list(APPEND STNL_PRIVATE_LIBS ${SYSTEM_BOOST_UUID})
    else()
        message(STATUS "Boost::uuid not found; skipping linking against it (header-only usage assumed or install boost-uuid).")
    endif()
endif()

target_link_libraries(stnl
    PUBLIC
        ${STNL_PUBLIC_LIBS}
    PRIVATE
        ${STNL_PRIVATE_LIBS}
)

# Ensure the final library name is set for installation/export if necessary
set_target_properties(stnl PROPERTIES EXPORT_NAME stnl)