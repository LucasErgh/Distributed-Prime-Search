#pragma once

#include "config.h"
#include "ServerLogic.h"

// Winsock Libraries
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <mutex>
#include <thread>
#include <chrono>

#include <memory>
#include <vector>
#include <utility>
#include <stdexcept>

namespace PrimeProcessor{

    class SocketManager {
    private:

        class ClientHandler{
        public:
            SocketManager* manager;

            SOCKET clientSocket;

            int recvbuflen = DEFAULT_BUFLEN;
            char recvbuf[DEFAULT_BUFLEN];
            int iSendResult;
            int iResult;

        public:

            ClientHandler(SOCKET& s, SocketManager* m);

            void clientComs();

            static int nextKey;
            int key;
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
            void createSocket();
            void startListening();
        };
        
        typedef std::vector<std::pair<std::shared_ptr<ClientHandler>, std::thread>> ClientList;
        ClientList clientList; // all client actively connected
        std::mutex clientListMutex;

        Listener listener;

        WSAData wsaData;

        // Called by Listener to add ClientSocket to clientList
        void addClient(SOCKET& c);

        ServerLogic *manager;
        range getRange() {}

    public:
        SocketManager(ServerLogic*);
        ~SocketManager();
        
        void start(){
            try { listener.createSocket(); }
            catch (const std::runtime_error& e) { throw e; } // propogate error
            
            try { listener.startListening(); } // start listener thread
            catch (const std::runtime_error& e) { throw e; } // propogate error
        }

        void stop(){
            // To-Do
        }

        // friends
        friend Listener;
        friend ClientHandler;
    };

    
}