set(Includes 
    "ServerLogic.h"
    "FileIO.h"
    "ServerInterface.h"
    "BoostNetworkManager.h"
)

add_library(includeServer INTERFACE ${Includes})

target_include_directories(includeServer INTERFACE ${asio_SOURCE_DIR}/asio/include)
