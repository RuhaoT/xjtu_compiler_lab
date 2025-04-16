cmake_minimum_required(VERSION 3.15)

project(Lab1_DFA_simulator)

# 查找yaml-cpp库
find_package(yaml-cpp REQUIRED)

# 包含头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 添加源文件
set(SOURCES
    dfa_simulator.cpp
    yaml_config_frontend.cpp
    standard_dfa_simulator.cpp
)

# 构建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接yaml-cpp库
target_link_libraries(${PROJECT_NAME} yaml-cpp)

