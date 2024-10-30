cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

project(DPPServer LANGUAGES CXX)

add_executable(${PROJECT_NAME} "src/DPPServer.cpp")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "DPPServer")

add_subdirectory(include)
add_subdirectory(src)
target_link_libraries(${PROJECT_NAME} include)
target_link_libraries(${PROJECT_NAME} src)