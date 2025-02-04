cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

project(Client LANGUAGES CXX)

add_executable(Client "src/DPPClient.cpp")
set_target_properties(Client PROPERTIES OUTPUT_NAME "DPPClient")

add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(Client PRIVATE includeClient)
target_link_libraries(Client PRIVATE srcClient)
target_link_libraries(Client PRIVATE CommonLib)
target_link_libraries(Client PRIVATE ws2_32)
