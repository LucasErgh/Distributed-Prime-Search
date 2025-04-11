#include "ServerLogic.h"
#include "MySockets.h"

#include <iostream>
#include <string>

int main() {
    using namespace PrimeProcessor;

    ServerLogic server;
    server.start();

    SocketManager socketManager(server);
    socketManager.start();

    std::cin.get();

    socketManager.stop();
    server.stop();

    return 0;
}
