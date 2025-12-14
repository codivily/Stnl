cmake_minimum_required(VERSION 3.8...3.31)

project(Sentinel)

# Set modern C++ and project-wide compile options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_CPPCOREGUIDELINES "Enable clang-tidy/format checks and stricter warnings to help follow the CppCoreGuidelines" ON)

if(MSVC)
	add_compile_options(/W4 /permissive-)
else()
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Locate clang-format/clang-tidy (optional)
find_program(CLANG_FORMAT_EXE NAMES clang-format-15 clang-format-14 clang-format-13 clang-format)
find_program(CLANG_TIDY_EXE NAMES clang-tidy-15 clang-tidy-14 clang-tidy-13 clang-tidy)

if(CLANG_TIDY_EXE AND ENABLE_CPPCOREGUIDELINES)
	message(STATUS "Enabling clang-tidy integration: ${CLANG_TIDY_EXE}")
	set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")
endif()

if(CLANG_FORMAT_EXE)
	file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
		${CMAKE_SOURCE_DIR}/stnl/*.cpp
		${CMAKE_SOURCE_DIR}/stnl/*.hpp
		${CMAKE_SOURCE_DIR}/stnl/**/*.cpp
		${CMAKE_SOURCE_DIR}/stnl/**/*.hpp
		${CMAKE_SOURCE_DIR}/main/*.cpp
		${CMAKE_SOURCE_DIR}/main/*.hpp
		${CMAKE_SOURCE_DIR}/main/**/*.cpp
		${CMAKE_SOURCE_DIR}/main/**/*.hpp
	)

	add_custom_target(format
		COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_CXX_SOURCE_FILES}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Run clang-format on project sources")

	add_custom_target(format-check
		COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${ALL_CXX_SOURCE_FILES}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Check formatting (clang-format --dry-run)")
endif()
# Find required packages
find_package(Boost COMPONENTS headers filesystem json url)

# Find vcpkg packages
find_package(libpqxx CONFIG REQUIRED)
find_package(PostgreSQL REQUIRED)  # If not auto-pulled by vcpkg

# Add subdirectories
add_subdirectory(stnl)
add_subdirectory(main)
