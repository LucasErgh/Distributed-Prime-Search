CMAKE_MINIMUM_REQUIRED(VERSION 3.30.0 FATAL_ERROR)

project(DPP LANGUAGES CXX)

message(STATUS "Directory for root CMakeList: ${CMAKE_CURRENT_SOURCE_DIR}/include")
set(CMAKE_BUILD_TYPE="DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")
set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(Benchmarks)
add_subdirectory(Common)
add_subdirectory(Server)
add_subdirectory(Client)

add_custom_target(run_benchmarks
    COMMENT "Run benchmarks"
    COMMAND $<TARGET_FILE:Benchmarks>
    DEPENDS Server Common Client
    USES_TERMINAL
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(run_server
    COMMENT "Run server application"
    COMMAND $<TARGET_FILE:DPPServer>
    DEPENDS Server Common
    USES_TERMINAL
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(run_client
    COMMENT "Run client application"
    COMMAND $<TARGET_FILE:DPPClient>
    DEPENDS Client Common
    USES_TERMINAL
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_custom_target(run_clients_parallel
        COMMENT "Run four client applications"
        COMMAND echo a
        COMMAND start $<TARGET_FILE:Client>
        COMMAND start $<TARGET_FILE:Client>
        COMMAND start $<TARGET_FILE:Client>
        COMMAND start $<TARGET_FILE:Client>
        DEPENDS Client Common
        USES_TERMINAL
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    add_custom_target(kill_clients
        COMMAND taskkill /F /IM DPPClient.exe /T
    )
else()
    message(WARNING "Command only configured on windows")
endif()
