cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

message(STATUS "Source Dir directories for Server: ${CMAKE_SOURCE_DIR}")

project(Server LANGUAGES CXX)

add_executable(Server "src/DPPServer.cpp")
set_target_properties(Server PROPERTIES OUTPUT_NAME "DPPServer")

target_include_directories(Server PRIVATE include)
file(GLOB srcServer "./src/*.cpp")
target_sources(Server PUBLIC ${srcServer})

target_link_libraries(Server PRIVATE CommonLib)
target_link_libraries(Server PRIVATE ws2_32)
