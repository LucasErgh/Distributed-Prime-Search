cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

message(STATUS "Source Dir directories for Server: ${CMAKE_SOURCE_DIR}")

add_library(Server STATIC)

set(DATA_DIR "${CMAKE_SOURCE_DIR}/data")
file(MAKE_DIRECTORY ${DATA_DIR})
add_compile_definitions(Primes_Found_Path="${DATA_DIR}/Primes_Found.txt")
add_compile_definitions(Primes_Searched_Path="${DATA_DIR}/Primes_Searched.txt")

file(GLOB srcServer "src/*.cpp")
target_sources(Server PRIVATE ${srcServer})
target_include_directories(Server PUBLIC include)

target_link_libraries(Server PUBLIC CommonLib)
target_link_libraries(Server PUBLIC ws2_32)

add_executable(DPPServer "src/DPPServer.cpp")
set_target_properties(DPPServer PROPERTIES OUTPUT_NAME "DPPServer")
target_link_libraries(DPPServer PRIVATE Server)

add_custom_target(run_server
    COMMENT "Run server application"
    COMMAND $<TARGET_FILE:DPPServer>
    DEPENDS Server CommonLib
    USES_TERMINAL
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
