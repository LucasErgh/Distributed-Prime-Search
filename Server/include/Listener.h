#ifndef LISTENER_H
#define LISTENER_H

#include "config.h"
#include <functional>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

namespace PrimeProcessor{

    class Listener{
    private:
        std::function<void(SOCKET&)> addClientCallback;

        SOCKET listenerSocket = INVALID_SOCKET;
        SOCKET clientSocket = INVALID_SOCKET;

        addrinfo *result = nullptr, *ptr = nullptr, hints;
        int iResult;

        char recvbuff[DEFAULT_BUFLEN];
        int recvbuflen = DEFAULT_BUFLEN;

        std::atomic_bool closingConnection = false;

    public:
        Listener(std::function<void(SOCKET&)>);

        // tries to accept a connection
        void createSocket();

        // listens for incomming connections adding them to SocketManager list
        void startListening();

        // Closes listener socket 
        void closeConnection();
    };

}

#endif
