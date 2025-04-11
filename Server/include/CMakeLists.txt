set(Includes 
    "ServerLogic.h"
    "MySockets.h"
    "FileIO.h"
    "ServerInterface.h"
    "ClientHandler.h"
)

add_library(includeServer INTERFACE ${Includes})
