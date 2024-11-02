#include "MySockets.h"
#include "ServerLogic.h"

#include <iostream>

int main() {
    using namespace PrimeProcessor;

    ServerLogic server;
    server.start();

    return 0;
}

