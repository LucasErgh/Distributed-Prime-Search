#include "ServerLogic.h"
#include "MySockets.h"

#include <iostream>
#include <string>

int main() {
    using namespace PrimeProcessor;

    ServerLogic server;
    server.start();
    std::cin.get();
    server.stop();

    return 0;
}