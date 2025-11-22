cmake_minimum_required(VERSION 3.8...3.31)

project(Sentinel)

# Find required packages
find_package(Boost COMPONENTS headers filesystem json url)

# Find vcpkg packages
find_package(libpqxx CONFIG REQUIRED)
find_package(PostgreSQL REQUIRED)  # If not auto-pulled by vcpkg

# Add subdirectories
add_subdirectory(stnl)
add_subdirectory(main)
