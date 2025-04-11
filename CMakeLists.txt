CMAKE_MINIMUM_REQUIRED(VERSION 3.30.0 FATAL_ERROR)

project(DPP LANGUAGES CXX)

message(STATUS "Directory for root CMakeList: ${CMAKE_CURRENT_SOURCE_DIR}/include")
set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(Common)
add_subdirectory(Server)
add_subdirectory(Client)

add_custom_target(run_server
    COMMENT "Run server application"
    COMMAND $<TARGET_FILE:Server>
    DEPENDS Server
    USES_TERMINAL
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(run_client
    COMMENT "Run server application"
    COMMAND $<TARGET_FILE:Client>
    DEPENDS Server
    USES_TERMINAL
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
