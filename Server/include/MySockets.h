#ifndef MySockets_h
#define MySockets_h

#include "Listener.h"
#include "ClientHandler.h"
#include "ServerInterface.h"
#include "Serialization.h"

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

    class ClientHandler;

    class SocketManager{
    public:

        SocketManager(ServerInterface& server);
        ~SocketManager();
        
        void start();

        void stop();

        std::array<unsigned long long, 2> requestWork() { return server.requestWork(); }
        void foundPrimes(std::vector<unsigned long long> p, std::array<unsigned long long, 2> r) { server.primesReceived(p, r); }
        void searchFailed(std::array<unsigned long long, 2>);

        // Called by Listener to add ClientSocket to clientList
        void addClient(SOCKET& c);

    private:
        std::atomic<bool> closingSocketManager = false;

        std::vector<std::pair<std::shared_ptr<ClientHandler>, std::thread>> clientList; // all client actively connected
        std::mutex clientListMutex;

        std::unique_ptr<Listener> listener;
        std::thread listenThread;

        WSAData wsaData;

        // removes a client from list of workers by key
        void removeClient(int key);
        // To-Do determine if I need to overload the removeClient function
        // void removeClient(std::shared_ptr<ClientHandler>);

        ServerInterface& server;

        void threadClosingLoop();
        std::thread clientClosingThread;
        std::mutex clientCloseMutex;

    public:
        std::condition_variable closeClientCondition;
        std::atomic<int> clientsToClose = 0;

    };
}

#endif
