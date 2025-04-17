set(Includes 
    "ServerLogic.h"
    "MySockets.h"
    "FileIO.h"
    "ServerInterface.h"
    "ClientHandler.h"
    "Listener.h"
    "BoostNetworkManager.h"
)

add_library(includeServer INTERFACE ${Includes})
