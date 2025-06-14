cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

project(Client LANGUAGES CXX)

add_executable(Client "src/DPPClient.cpp")
set_target_properties(Client PROPERTIES OUTPUT_NAME "DPPClient")

file(GLOB srcClient "src/*.cpp")
target_sources(Client PUBLIC ${srcClient})
target_include_directories(Client PRIVATE "include")

target_link_libraries(Client PRIVATE CommonLib)
target_link_libraries(Client PRIVATE ws2_32)
