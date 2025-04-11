#ifndef LISTENER_H
#define LISTENER_H

#include "MySockets.h"
#include "config.h"
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

namespace PrimeProcessor{

    class SocketManager;

    class Listener{
    public:
        SocketManager* manager;
        
        SOCKET listenerSocket = INVALID_SOCKET;
        SOCKET clientSocket = INVALID_SOCKET;
        
        addrinfo *result = nullptr, *ptr = nullptr, hints;
        int iResult;

        char recvbuff[DEFAULT_BUFLEN];
        int recvbuflen = DEFAULT_BUFLEN;

        std::atomic_bool closingConnection = false;

    public:
        Listener();
        
        // tries to accept a connection
        void createSocket();

        // listens for incomming connections adding them to SocketManager list
        void startListening();

        // Closes listener socket 
        void closeConnection();
    };
}

#endif
