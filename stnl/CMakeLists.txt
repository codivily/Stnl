# STNL Library
add_library(stnl
    STATIC
        src/utils.cpp
        src/logger.cpp
        src/stnl_module.cpp
        src/session.cpp
        src/server.cpp
        src/request.cpp
        src/middleware.cpp
        src/config.cpp
        src/column.cpp
        src/blueprint.cpp
        src/migrator.cpp
)

# Include directories (public for users of the lib)
target_include_directories(stnl
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        src
)

# Link dependencies
target_link_libraries(stnl
    PUBLIC
        Boost::headers
    PRIVATE
        Boost::json
        Boost::url
        libpqxx::pqxx
)