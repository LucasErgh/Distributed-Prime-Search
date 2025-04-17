#include "ServerLogic.h"
#include "MySockets.h"
#include "NetworkManager.h"

#include <iostream>
#include <string>

int main() {
    using namespace PrimeProcessor;

    ServerLogic server;
    server.start();

    NetworkManager networkManager(server);

    try {
        networkManager.start();
        // Keep the server running or add logic to handle shutdown
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    networkManager.stop();
    server.stop();

    return 0;
}
