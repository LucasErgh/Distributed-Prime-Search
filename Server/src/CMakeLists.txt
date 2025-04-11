set(Dirs
    "../include"
    ${CMAKE_SOURCE_DIR}/Common/include
)

set(Sources 
    "DPPServer.cpp"
    "MySockets.cpp"
    "ServerLogic.cpp"
    "FileIO.cpp"
    "ServerInterface.cpp"
)

add_library(srcServer ${Sources})
target_include_directories(srcServer PUBLIC ${Dirs})
