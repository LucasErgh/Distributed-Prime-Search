cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

message(STATUS "Source Dir directories for Server: ${CMAKE_SOURCE_DIR}")

project(Server LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-34-2
)
FetchContent_MakeAvailable(asio)

add_executable(Server "src/DPPServer.cpp")
set_target_properties(Server PROPERTIES OUTPUT_NAME "DPPServer")

add_subdirectory(include)
add_subdirectory(src)

target_include_directories(Server PRIVATE ${asio_SOURCE_DIR}/asio/include)

target_link_libraries(Server PRIVATE includeServer)
target_link_libraries(Server PRIVATE srcServer)
target_link_libraries(Server PRIVATE CommonLib)
