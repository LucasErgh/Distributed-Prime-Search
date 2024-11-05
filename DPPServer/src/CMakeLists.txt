set(Dirs
    "../include"
)

set(Sources 
    "DPPServer.cpp"
    "MySockets.cpp"
    "ServerLogic.cpp"
    "Serialization.cpp"
)

add_library(src ${Sources})
target_include_directories(src PUBLIC ${Dirs})