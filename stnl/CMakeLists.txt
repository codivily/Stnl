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
  src/db/sp_param.cpp
  src/db/sp_blueprint.cpp
  src/db/migration.cpp
  src/db/migrator.cpp
  src/db/column.cpp
  src/db/inserter.cpp
  src/db/connection_pool.cpp
)

# This maximizes parallel compilation and greatly improves incremental builds.
list(LENGTH STNL_SOURCES STNL_SOURCES_COUNT)
message(STATUS "Creating ${STNL_SOURCES_COUNT} static library components for STNL.")

set(STNL_COMPONENT_LIBS "") # List to collect all component library names

foreach(SOURCE_FILE ${STNL_SOURCES})
    # Extract filename without extension (e.g., 'src/utils.cpp' -> 'stnl_utils')
    get_filename_component(BASE_NAME ${SOURCE_FILE} NAME_WE)
    set(LIB_NAME "stnl_${BASE_NAME}")

    # Create the small static library
    add_library(${LIB_NAME} STATIC ${SOURCE_FILE})

    # Apply all required includes to the component library
    target_include_directories(${LIB_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
            src
    )

    # Set Windows SDK version for Boost/Asio compatibility (Windows 7 target)
    target_compile_definitions(${LIB_NAME} 
        PUBLIC 
            _WIN32_WINNT=0x0601
    )

    # Apply all required dependencies to the component library
    # Note: Dependencies must be linked to the compilation unit that uses them.
    target_link_libraries(${LIB_NAME}
        PUBLIC
            Boost::headers
        PRIVATE
            Boost::json
            Boost::url
            libpqxx::pqxx
    )
    
    # Add the new library name to the list
    list(APPEND STNL_COMPONENT_LIBS ${LIB_NAME})
endforeach()


# ----------------------------------------------------
# 2. Final STNL Interface Library
# ----------------------------------------------------
# This is the single target that consumers will link against. 

# We use an INTERFACE library here. This target itself won't be compiled, 
# but simply tells CMake that anyone linking against 'stnl' must also link
# against all the component libraries and inherit the dependencies.
add_library(stnl INTERFACE)

# Propagate the public includes to consumers that link 'stnl'
target_include_directories(stnl
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# FIX: Use set_property for compile definitions on the INTERFACE target.
# Propagate the Windows target definition to any consumer of this library
set_property(TARGET stnl 
    APPEND PROPERTY 
    INTERFACE_COMPILE_DEFINITIONS "_WIN32_WINNT=0x0601"
)

# Aggregate all the component libraries and all dependencies
target_link_libraries(stnl
    INTERFACE
        ${STNL_COMPONENT_LIBS}
        Boost::headers
        Boost::json
        Boost::url
        libpqxx::pqxx
)

# Ensure the final library name is set for installation/export if necessary
set_target_properties(stnl PROPERTIES EXPORT_NAME stnl)