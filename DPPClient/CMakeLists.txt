cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

project(DPPClient LANGUAGES CXX)

add_executable(${PROJECT_NAME} "src/DPPClient.cpp")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "DPPClient")

add_subdirectory(include)
add_subdirectory(src)
target_link_libraries(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE src)

target_link_libraries(DPPClient PRIVATE ws2_32)