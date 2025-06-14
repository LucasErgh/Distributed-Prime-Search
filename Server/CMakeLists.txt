cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

message(STATUS "Source Dir directories for Server: ${CMAKE_SOURCE_DIR}")

add_library(Server STATIC)

file(GLOB srcServer "src/*.cpp")
target_sources(Server PRIVATE ${srcServer})
target_include_directories(Server PUBLIC include)

target_link_libraries(Server PUBLIC CommonLib)
target_link_libraries(Server PUBLIC ws2_32)

add_executable(DPPServer "src/DPPServer.cpp")
set_target_properties(DPPServer PROPERTIES OUTPUT_NAME "DPPServer")
target_link_libraries(DPPServer PRIVATE Server)
