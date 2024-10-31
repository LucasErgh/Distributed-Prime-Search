#include "MySockets.h"
#include "ServerLogic.h"

// Winsock Libraries
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <iostream>

#include <memory>

int main() {
    using namespace PrimeProcessor;

    ServerLogic server;
    server.start();

    return 0;
}

