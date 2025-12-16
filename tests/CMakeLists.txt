cmake_minimum_required(VERSION 3.20)

# Test executables
add_executable(test_logger test_logger.cpp)

# Link against the main library
target_link_libraries(test_logger PRIVATE stnl)

# Set C++ standard
target_compile_features(test_logger PRIVATE cxx_std_20)

# Include directories
target_include_directories(test_logger PRIVATE 
    ${CMAKE_SOURCE_DIR}/stnl/include
)

# Optional: Enable testing with CTest
enable_testing()
add_test(NAME LoggerTest COMMAND test_logger)
