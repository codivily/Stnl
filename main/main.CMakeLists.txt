# Main executable

set(SOURCES
  src/main.cpp
  src/modules/app/app.cpp
  src/modules/ticker/ticker.cpp
)

# Headers directory
include_directories(include src)

add_executable(app ${SOURCES})

# Link to STNL lib
target_link_libraries(app
  PUBLIC
    Boost::headers
  PRIVATE
    Boost::filesystem
    Boost::json
    Boost::url
    libpqxx::pqxx
    stnl
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