set(Dirs
    "../include"
    "${CMAKE_SOURCE_DIR}/Common/include"
)

set(Sources 
    "DPPClient.cpp"
    "ConnectionHandler.cpp"
    "PrimeSearcher.cpp"
)

add_library(srcClient ${Sources})
target_include_directories(srcClient PUBLIC ${Dirs})