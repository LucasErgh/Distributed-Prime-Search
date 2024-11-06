CMAKE_MINIMUM_REQUIRED(VERSION 3.30.0 FATAL_ERROR)

project(DPP LANGUAGES CXX)

message(STATUS "Directory for root CMakeList: ${CMAKE_CURRENT_SOURCE_DIR}/include")
set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(Common)
add_subdirectory(Server)
add_subdirectory(Client)
