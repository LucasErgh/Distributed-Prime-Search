#pragma once

#include "config.h"

// Winsock Libraries
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib") // Makes compiler include this library

#include <vector>

namespace MySockets{

    class SocketManager {
    public:

        class ClientHandler{
            private: 
                SOCKET clientSocket;

                int recvbuflen = DEFAULT_BUFLEN;
                char recvbuf[DEFAULT_BUFLEN];
                int iSendResult;
                int iResult;

            public:
            ClientHandler(SOCKET& s);

            void clientComs();
        };

        class Listener{
            private:
                SocketManager* manager;
                
                SOCKET listenerSocket = INVALID_SOCKET;
                SOCKET clientSocket = INVALID_SOCKET;
                
                char recvbuff[DEFAULT_BUFLEN];
                int recvbuflen = DEFAULT_BUFLEN;

            public:
                Listener();
                
                // tries to accept a connection
                void startListening();
        };

        std::vector<ClientHandler> clientList; // all client actively connected
        Listener listener;

        // Called by Listener to add ClientSocket to clientList
        void addClient(SOCKET& c);

    public:

        class WSAStartupFailed {};

        SocketManager();
        ~SocketManager();
        

    };
}