cmake_minimum_required(VERSION 3.8...3.31)

project(Sentinel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Boost COMPONENTS headers filesystem beast)

# Add subdirectories
add_subdirectory(stnl)
add_subdirectory(main)