# Main executable
add_executable(app
  main.cpp
  server_main.cpp
  ticker.cpp
)


# Link to STNL lib
target_link_libraries(app
  PUBLIC
    Boost::headers
  PRIVATE
    Boost::filesystem
    stnl
)