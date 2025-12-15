# Main executable

set(SOURCES
  src/main.cpp
  src/modules/app/app.cpp
  src/modules/ticker/ticker.cpp
)

# Headers directory
include_directories(include src)

add_executable(app ${SOURCES})

if(ENABLE_PCH)
    target_precompile_headers(app REUSE_FROM stnl)
endif()

# Link to STNL lib
set(APP_PUBLIC_LIBS
  Boost::headers
)

set(APP_PRIVATE_LIBS
  Boost::filesystem
  Boost::json
  Boost::url
  libpqxx::pqxx
  stnl
)

# Conditionally add Boost::dll when available (may not be present on some installs)
if(TARGET Boost::dll)
  list(APPEND APP_PRIVATE_LIBS Boost::dll)
elseif(DEFINED Boost_DLL_LIBRARY AND Boost_DLL_LIBRARY)
  list(APPEND APP_PRIVATE_LIBS ${Boost_DLL_LIBRARY})
else()
  # Try common system library names
  find_library(SYSTEM_BOOST_DLL NAMES boost_dll libboost_dll HINTS ${CMAKE_LIBRARY_PATH} PATH_SUFFIXES lib)
  if(SYSTEM_BOOST_DLL)
    list(APPEND APP_PRIVATE_LIBS ${SYSTEM_BOOST_DLL})
  else()
    message(STATUS "Boost::dll not found; proceeding without it (functionality using Boost.DLL will be unavailable).")
  endif()
endif()

target_link_libraries(app
  PUBLIC
    ${APP_PUBLIC_LIBS}
  PRIVATE
    ${APP_PRIVATE_LIBS}
)

# Copy the config.json file to the build directory
add_custom_command(
  TARGET app
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
  "$<TARGET_FILE_DIR:app>/config.json"
  COMMENT "Copying config.json file to build directory"
)