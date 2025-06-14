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

add_custom_target(run_client
    COMMENT "Run client application"
    COMMAND $<TARGET_FILE:DPPClient>
    DEPENDS Client CommonLib
    USES_TERMINAL
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_custom_target(run_clients_parallel
        COMMENT "Run four client applications"
        COMMAND start $<TARGET_FILE:DPPClient>
        COMMAND start $<TARGET_FILE:DPPClient>
        COMMAND start $<TARGET_FILE:DPPClient>
        COMMAND start $<TARGET_FILE:DPPClient>
        DEPENDS Client CommonLib
        USES_TERMINAL
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    add_custom_target(kill_clients
        COMMAND taskkill /F /IM DPPClient.exe /T
    )
else()
    message(WARNING "Command only configured on windows")
endif()
