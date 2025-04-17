#include "ServerLogic.h"
#include "BoostNetworkManager.h"

#include <iostream>
#include <string>

int main() {
    using namespace PrimeProcessor;

    ServerLogic server;
    server.start();

    BoostManager socketManager(server);
    socketManager.start();

    std::cin.get();

    socketManager.stop();
    server.stop();

    return 0;
}
