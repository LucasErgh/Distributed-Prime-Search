#pragma once

#include "config.h"

// Winsock Libraries
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <mutex>
#include <thread>

#include <memory>
#include <vector>
#include <utility>

namespace MySockets{

    class SocketManager {
    public:

        class ClientHandler{
            public:
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
            public:
                SocketManager* manager;
                
                SOCKET listenerSocket = INVALID_SOCKET;
                SOCKET clientSocket = INVALID_SOCKET;
                
                addrinfo *result = nullptr, *ptr = nullptr, hints;
                int iResult;

                char recvbuff[DEFAULT_BUFLEN];
                int recvbuflen = DEFAULT_BUFLEN;

            public:
                Listener();
                
                // tries to accept a connection
                void startListening();
        };

        std::vector<std::pair<std::shared_ptr<ClientHandler>, std::thread>> clientList; // all client actively connected
        std::mutex clientListMutex;

        Listener listener;

        WSAData wsaData;

        // Called by Listener to add ClientSocket to clientList
        void addClient(SOCKET& c);

    public:
        friend Listener;
        friend ClientHandler;

        class WSAStartupFailed {};

        SocketManager(WSAData);
        ~SocketManager();
        
        void start(){
            listener.startListening(); // start listener thread
        }
    };
}