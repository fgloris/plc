set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.10)

project(plc)

add_executable(${CMAKE_PROJECT_NAME} src/main.cpp src/keyword.cpp src/grammar.cpp src/ast.cpp src/asm.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)