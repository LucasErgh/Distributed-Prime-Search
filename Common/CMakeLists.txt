# Define the Common library
add_library(CommonLib STATIC 
    src/Serialization.cpp
)

target_include_directories(CommonLib
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)