cmake_minimum_required(VERSION 3.14)
project(Lab2_Lexical_analyzer)

# package config
find_package(PkgConfig REQUIRED)
# 查找yaml-cpp库
find_package(yaml-cpp REQUIRED)
# Add find_package for spdlog
find_package(spdlog REQUIRED)
# reflex for lexer, it does not have CMake support
pkg_check_modules(reflex REQUIRED IMPORTED_TARGET reflex) # Corrected module name and keyword order

# include
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip # Or specific commit/tag
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing() # Enable CTest support

# pathmaking for test
set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test/data)

set(TEST_DATA_DEST ${CMAKE_CURRENT_BINARY_DIR}/test/data)

add_executable(test_all
    test/dfa_simulator_tests.cpp
    test/lexer_factory_tests.cpp
    test/test_environment_setup.cpp
    test/dfa_based_lexer_tests.cpp
    test/flex_based_lexer_tests.cpp
    test/dfa_config_tests.cpp
    include/dfa_based_lexer.cpp
    include/flex_based_lexer.cpp
    include/standard_dfa_simulator.cpp
    include/yaml_lexer_factory.cpp
)

target_link_libraries(test_all
    gtest_main
    gtest
    spdlog::spdlog # Link against the imported target spdlog::spdlog
    yaml-cpp
    PkgConfig::reflex
)

# Copy test data to the build directory
add_custom_command(
    TARGET test_all
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${TEST_DATA_DIR}
    ${TEST_DATA_DEST}
)

include(GoogleTest)
gtest_discover_tests(test_all) # Automatically find tests

# Add the frontend executable
add_executable(frontend
    frontend.cpp
    # Include source files needed by the frontend (factory, lexer implementation, etc.)
    include/yaml_lexer_factory.cpp
    include/dfa_based_lexer.cpp
    include/flex_based_lexer.cpp
    include/standard_dfa_simulator.cpp
)

# Link frontend against required libraries
target_link_libraries(frontend PRIVATE
    yaml-cpp
    spdlog::spdlog
    PkgConfig::reflex
)