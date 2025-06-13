cmake_minimum_required(VERSION 3.14)
project(Lab4_SLR1_Parsing_Table_Generator)

# yaml-cpp
find_package(yaml-cpp REQUIRED)
# Add find_package for spdlog
find_package(spdlog REQUIRED)
# boost graph library for visualization
find_package(Boost REQUIRED COMPONENTS graph)
# tabulate library for visualization
find_package(tabulate REQUIRED)

# package config
find_package(PkgConfig REQUIRED)
# reflex for lexer, it does not have CMake support
pkg_check_modules(reflex REQUIRED IMPORTED_TARGET reflex) # Corrected module name and keyword order

# --- Global Include Directory ---
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# headers are in include/ subdirectories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/cfg)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/fsm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/parsing_table)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/lexer)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/syntax_semantic_analyzer)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/tree)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/common)

# --- Component Libraries ---

# CFG Utilities Library
add_library(cfg_utils STATIC
    src/cfg/yaml_cfg_loader.cpp
    src/cfg/cfg_analyzer.cpp
)
target_link_libraries(cfg_utils PUBLIC
    yaml-cpp # yaml_cfg_loader.cpp needs this
)
# Headers for this library are in include/cfg/

# FSM Utilities Library
add_library(fsm_utils STATIC
    src/fsm/nfa_model.cpp
    src/fsm/multitype_dfa_simulator.cpp
    src/fsm/standard_nfa_dfa_converter.cpp
    src/fsm/yaml_dfa_config_frontend.cpp
    src/fsm/standard_dfa_simulator.cpp
)
# Headers for this library are in include/fsm/

# Lexer Utilities Library
add_library(lexer_utils STATIC
    src/lexer/yaml_lexer_factory.cpp
    src/lexer/dfa_based_lexer.cpp
    src/lexer/flex_based_lexer.cpp
    src/lexer/token_loader.cpp
)
# this library requires fsm_utils and cfg_utils
target_link_libraries(lexer_utils PUBLIC
    fsm_utils
    cfg_utils
)
# require yaml-cpp and reflex
target_link_libraries(lexer_utils PUBLIC
    yaml-cpp
    PkgConfig::reflex # Corrected to use PkgConfig for reflex
)

# Parsing Table Generation Utilities Library
add_library(parsing_table_utils STATIC
    src/parsing_table/itemset_generator.cpp
    src/parsing_table/itemset_to_parsing_table.cpp
    src/parsing_table/simple_lr_parsing_table_generator.cpp
    src/parsing_table/lr_parsing_model.cpp
    src/parsing_table/slr1_conflict_resolver.cpp
    src/parsing_table/slr1_parsing_table_generator.cpp
    src/parsing_table/lr1_parsing_table_generator.cpp
)
# Headers for this library are in include/parsing_table/
# this library requires fsm_utils and cfg_utils
target_link_libraries(parsing_table_utils PUBLIC
    fsm_utils
    cfg_utils
)

# Syntax Semantic Analyzer Utilities Library
add_library(syntax_semantic_analyzer_utils STATIC
    src/syntax_semantic_analyzer/semantic_loader.cpp
    src/syntax_semantic_analyzer/syntax_semantic_model.cpp
    src/syntax_semantic_analyzer/scope_table.cpp
    src/syntax_semantic_analyzer/symbol_table.cpp
    src/syntax_semantic_analyzer/ast_model.cpp
    src/syntax_semantic_analyzer/syntax_semantic_analyzer.cpp
)
# require all previous libraries except FSM
target_link_libraries(syntax_semantic_analyzer_utils PUBLIC
    cfg_utils
    parsing_table_utils
    lexer_utils
    yaml-cpp
)

# Visualization Utilities Library
add_library(viz_utils STATIC
    src/common/visualization_helper.cpp
)
# this library requires all previous libraries
target_link_libraries(viz_utils PUBLIC
    cfg_utils
    fsm_utils
    parsing_table_utils
    syntax_semantic_analyzer_utils
)
target_link_libraries(viz_utils PUBLIC
    Boost::graph
    tabulate::tabulate
)
# Headers for this library are in include/common/ (or include/viz/)

# Test Utilities Library
add_library(test_utils STATIC
    src/common/testing_utils.cpp
)
# this library requires all previous libraries
target_link_libraries(test_utils PUBLIC
    cfg_utils
    fsm_utils
    parsing_table_utils
    lexer_utils
    viz_utils
    gtest
)


# --- GoogleTest ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing() # Enable CTest support

# --- Test Executable ---
set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test/data)
set(TEST_DATA_DEST ${CMAKE_CURRENT_BINARY_DIR}/test/data)

add_executable(test_all
    test/cfg/cfg_analyzer_tests.cpp
    test/cfg/yaml_cfg_loader_tests.cpp
    test/fsm/multitype_dfa_simulator_tests.cpp
    test/fsm/dfa_simulator_tests.cpp
    test/parsing_table/itemset_generator_tests.cpp
    test/parsing_table/itemset_to_parsing_table_tests.cpp
    test/parsing_table/simple_lr_parsing_table_generator_tests.cpp
    test/parsing_table/slr1_parsing_table_generator_tests.cpp
    test/parsing_table/lr1_parsing_table_generator_tests.cpp
    test/common/visualization_helper_tests.cpp
    test/lexer/yaml_lexer_factory_tests.cpp
    test/lexer/dfa_based_lexer_tests.cpp
    test/lexer/flex_based_lexer_tests.cpp
    test/lexer/yaml_factory_dfa_config_tests.cpp
    test/lexer/token_loader_tests.cpp
    test/syntax_semantic_analyzer/semantic_loader_tests.cpp
    test/syntax_semantic_analyzer/scope_table_tests.cpp
    test/syntax_semantic_analyzer/symbol_table_tests.cpp
    test/syntax_semantic_analyzer/syntax_semantic_analyzer_tests.cpp
)

target_link_libraries(test_all PRIVATE
    cfg_utils
    fsm_utils
    parsing_table_utils
    lexer_utils
    syntax_semantic_analyzer_utils
    viz_utils
    test_utils
    gtest_main
    gtest
    spdlog::spdlog # Assuming tests might use logging
    # yaml-cpp is linked via cfg_utils
)

add_custom_command(
    TARGET test_all
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${TEST_DATA_DIR}
    ${TEST_DATA_DEST}
    COMMENT "Copying test data for test_all"
)

include(GoogleTest)
gtest_discover_tests(test_all)

# --- Main Frontend Executable ---
add_executable(frontend
    frontend.cpp
    # frontend.cpp links against component libraries
)

target_link_libraries(frontend PRIVATE
    cfg_utils
    fsm_utils
    parsing_table_utils
    lexer_utils
    syntax_semantic_analyzer_utils
    viz_utils
    spdlog::spdlog
    # yaml-cpp, Boost::graph, tabulate::tabulate are linked via component libs
)

# Optional: Set C++ standard if not already set elsewhere
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED True)