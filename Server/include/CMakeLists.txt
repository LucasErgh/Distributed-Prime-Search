set(Includes 
    "ServerLogic.h"
    "MySockets.h"
    "FileIO.h"
    "ClientHandler.h"
    "Listener.h"
    "MessageQueue.h"
)

add_library(includeServer INTERFACE ${Includes})
