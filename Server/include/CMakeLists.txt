set(Includes 
    "ServerLogic.h"
    "MySockets.h"
    "FileIO.h"
    "ServerInterface.h"
)

add_library(includeServer INTERFACE ${Includes})
