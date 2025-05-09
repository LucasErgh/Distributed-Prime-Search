#ifndef MySockets_h
#define MySockets_h

#include "Listener.h"
#include "ClientHandler.h"
#include "MessageQueue.h"
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

    class SocketManager{
    public:

        SocketManager(MessageQueue* messageQueue);
        ~SocketManager();
        
        void start();

        void stop();

        void clientDisconnected();

        // Called by Listener to add ClientSocket to clientList
        void addClient(SOCKET& c);

    private:
        MessageQueue* messageQueue;

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

        void threadClosingLoop();
        std::thread clientClosingThread;
        std::mutex clientCloseMutex;

    public:
        std::condition_variable closeClientCondition;
        std::atomic<int> clientsToClose = 0;

    };

}

#endif
