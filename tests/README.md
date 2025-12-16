# Tests Directory

This directory contains tests for the STNL project.

## Building Tests

Tests are automatically built when you build the main project:

```bash
cmake --build --preset=linux
```

The test executables will be in `build/tests/`.

## Running Tests

### Run individual test:
```bash
./build/tests/test_logger
```

### Run all tests with CTest:
```bash
cd build
ctest
```

Or with verbose output:
```bash
cd build
ctest --verbose
```

## Available Tests

### test_logger
Tests the logging mechanism including:
- Single-line log messages
- Multi-line SQL queries with indentation
- Multi-line JSON with indentation
- Error stack traces
- Warning messages
- ANSI color coding for different log levels

## Adding New Tests

1. Create a new `.cpp` file in the `tests/` directory
2. Add it to `tests/CMakeLists.txt`:
   ```cmake
   add_executable(test_name test_name.cpp)
   target_link_libraries(test_name PRIVATE stnl)
   target_compile_features(test_name PRIVATE cxx_std_20)
   add_test(NAME TestName COMMAND test_name)
   ```
3. Rebuild the project

## Test Output Example

```
[12:30:15.123] INF   Server started successfully on port 3777
[12:30:15.124] DBG   Executing SQL query:
                    SELECT 
                        users.id,
                        users.name
                    FROM users
```
