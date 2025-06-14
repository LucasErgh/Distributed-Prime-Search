cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

add_library(Client STATIC)

file(GLOB srcClient "src/*.cpp")
target_sources(Client PRIVATE ${srcClient})
target_include_directories(Client PUBLIC "include")

target_link_libraries(Client PUBLIC CommonLib)
target_link_libraries(Client PUBLIC ws2_32)

add_executable(DPPClient "src/DPPClient.cpp")
set_target_properties(DPPClient PROPERTIES OUTPUT_NAME "DPPClient")
target_link_libraries(DPPClient PRIVATE Client)
