clang++ `
    .\Server\src\ServerLogic.cpp `
    .\Server\src\DPPServer.cpp `
    .\Server\src\MySockets.cpp `
    .\Common\src\FancyConsole.cpp `
    .\Common\src\Serialization.cpp `
    .\Client\src\DppClient.cpp `
    .\Client\src\PrimeSearcher.cpp `
    .\Client\src\ConnectionHandler.cpp `
    -I.\Server\include `
    -I.\Common\include `
    -I.\Client\include `
    -o DPPApp.exe `
    -std=c++20

