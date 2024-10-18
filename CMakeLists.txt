set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.10)

project(plc)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_LIST})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)