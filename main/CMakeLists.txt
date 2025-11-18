# Main executable

set(SOURCES
  src/main.cpp
  src/server_main.cpp
  src/ticker.cpp
)

# Headers directory
include_directories(include)

add_executable(app ${SOURCES})

# Link to STNL lib
target_link_libraries(app
  PUBLIC
    Boost::headers
  PRIVATE
    Boost::filesystem
    stnl
)