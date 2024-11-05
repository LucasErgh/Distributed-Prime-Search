cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

project(DPPServer LANGUAGES CXX)

add_executable(${PROJECT_NAME} "src/DPPServer.cpp")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "DPPServer")

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

add_subdirectory(include)
add_subdirectory(src)
target_link_libraries(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)