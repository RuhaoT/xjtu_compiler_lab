cmake_minimum_required(VERSION 3.14)
project(Lab3_LR0_Parsing_Table_Generator)

# yaml-cpp
find_package(yaml-cpp REQUIRED)
# Add find_package for spdlog
find_package(spdlog REQUIRED)
# boost graph library for visualization
find_package(Boost REQUIRED COMPONENTS graph)
# tabulate library for visualization
find_package(tabulate REQUIRED)

# include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip # Or specific commit/tag
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing() # Enable CTest support

# pathmaking for test
set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test/data)

set(TEST_DATA_DEST ${CMAKE_CURRENT_BINARY_DIR}/test/data)

add_executable(test_all
    test/testing_utils.cpp
    test/yaml_cfg_loader_tests.cpp
    test/itemset_generator_tests.cpp
    test/multitype_dfa_simulator_tests.cpp
    test/itemset_to_parsing_table_tests.cpp
    test/simple_lr_parsing_table_generator_tests.cpp
    test/visualization_helper_tests.cpp
    include/nfa_model.cpp
    include/yaml_cfg_loader.cpp
    include/itemset_generator.cpp
    include/yaml_config_frontend.cpp
    include/multitype_dfa_simulator.cpp
    include/itemset_to_parsing_table.cpp
    include/standard_nfa_dfa_converter.cpp
    include/visualization_helper.cpp
    include/simple_lr_parsing_table_generator.cpp
    include/lr_parsing_model.cpp
)

add_executable(frontend
    frontend.cpp
    include/nfa_model.cpp
    include/yaml_cfg_loader.cpp
    include/itemset_generator.cpp
    include/yaml_config_frontend.cpp
    include/multitype_dfa_simulator.cpp
    include/itemset_to_parsing_table.cpp
    include/standard_nfa_dfa_converter.cpp
    include/visualization_helper.cpp
    include/simple_lr_parsing_table_generator.cpp
    include/lr_parsing_model.cpp
)

target_link_libraries(test_all
    gtest_main
    gtest
    spdlog::spdlog
    yaml-cpp
)

target_link_libraries(frontend
    spdlog::spdlog
    yaml-cpp
    Boost::graph
    tabulate::tabulate
)

add_custom_command(
    TARGET test_all
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${TEST_DATA_DIR}
    ${TEST_DATA_DEST}
)

include(GoogleTest)
gtest_discover_tests(test_all) # Automatically find tests