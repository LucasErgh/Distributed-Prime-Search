set(Dirs
    "../include"
)

set(Sources 
    "DPPClient.cpp"
    "ConnectionHandler.cpp"
    "PrimeSearcher.cpp"
    "Serialization.cpp"
)

add_library(src ${Sources})
target_include_directories(src PUBLIC ${Dirs})