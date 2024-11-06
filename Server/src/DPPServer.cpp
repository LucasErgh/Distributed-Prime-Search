#include "ServerLogic.h"
#include "MySockets.h"

#include <iostream>

int main() {
    using namespace PrimeProcessor;

    ServerLogic server;
    server.start();

    return 0;
}