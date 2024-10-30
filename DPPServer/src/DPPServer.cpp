#include "MySockets.h"
#include "ServerLogic.h"

// Winsock Libraries
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <iostream>

int main() {
    using namespace MySockets;

    // Initialize WinSock
    WSAData wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed: \n" << iResult;
        return 1;
    }

    // Create SocketManager
    SocketManager manager(wsaData);

    return 0;
}

