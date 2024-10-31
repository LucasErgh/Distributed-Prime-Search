#include "MySockets.h"
#include "ServerLogic.h"

// Winsock Libraries
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <iostream>

#include <memory>

int main() {
    using namespace MySockets;

    // using pointer so I can initialize it in try block without making move constructor
    std::unique_ptr<SocketManager> manager; 

    // Create SocketManager
    try { manager = std::make_unique<MySockets::SocketManager>(); }
    catch (SocketManager::WSAStartupFailed) { std::cout << "WSAStartup failed. Exiting program.\n"; }
    
    manager->start();

    return 0;
}

