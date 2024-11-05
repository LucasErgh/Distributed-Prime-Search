#ifndef MySockets_h
#define MySockets_h

#include "config.h"
#include "ServerLogic.h"
#include "Serialization.h"

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

    class SocketManager{
    private:

        class ClientHandler{
        public:
            SocketManager* manager;

            SOCKET clientSocket;

            uint8_t header[3];
            int iSendResult;
            int iResult;
            std::vector<std::byte> lastSent;
            // To-Do impliment stop function with public method
            bool stop = true;

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
        std::vector<std::byte> getRange() { return createMsg(manager->getRange()); }
        void foundPrimes(std::vector<unsigned long long> p) { manager->foundPrimes(p); }

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

#endif