#ifndef MySockets_h
#define MySockets_h

#include "ServerLogic.h"
#include "Serialization.h"
#include "config.h"

// Winsock Libraries
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

#include <memory>
#include <vector>
#include <utility>
#include <stdexcept>


namespace PrimeProcessor{

    namespace{

        class ClientHandler{
        public:
            SocketManager* manager;

            SOCKET clientSocket;

            uint8_t header[3];
            int iSendResult;
            int iResult;
            std::vector<std::byte> lastSent;
            std::array<unsigned long long, 2> lastRange;
            std::atomic_bool currentlyRunning = true;

        public:

            ClientHandler(SOCKET& s, SocketManager* m);

            // cloeses conenction with client
            void closeConnection();

            // loop that communicates with clients
            void clientComs();
            void commsFailed();

            static int nextKey;
            int key;

            std::atomic<bool> needsClosedByParent = false;
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

    class SocketManager{
    public:

        SocketManager(ServerLogic*);
        ~SocketManager();
        
        void start();

        void stop();

        // friends
        friend Listener;
        friend ClientHandler;

    private:
        std::atomic<bool> closingSocketManager = false;

        std::vector<std::pair<std::shared_ptr<ClientHandler>, std::thread>> clientList; // all client actively connected
        std::mutex clientListMutex;

        Listener listener;
        std::thread listenThread;

        WSAData wsaData;

        // Called by Listener to add ClientSocket to clientList
        void addClient(SOCKET& c);

        // removes a client from list of workers by key
        void removeClient(int key);
        // To-Do determine if I need to overload the removeClient function
        // void removeClient(std::shared_ptr<ClientHandler>);

        ServerLogic *manager;
        
        std::array<unsigned long long, 2> getRange() { return manager->getRange(); }

        void foundPrimes(std::vector<unsigned long long> p, std::array<unsigned long long, 2> r) { manager->foundPrimes(p, r); }
        
        // returns array client was searching to ServerLogic work queue
        void searchFailed(std::array<unsigned long long, 2>);

        void threadClosingLoop();
        std::atomic<int> clientsToClose = 0;
        std::thread clientClosingThread;
        std::condition_variable closeClientCondition;
        std::mutex clientCloseMutex;
    };   
}

#endif