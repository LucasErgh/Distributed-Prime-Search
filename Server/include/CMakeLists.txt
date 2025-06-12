set(Includes 
    "ServerLogic.h"
    "FileIO.h"
    "MessageQueue.h"
    "NetworkManager.h"
)

add_library(includeServer INTERFACE ${Includes})
