#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "ServerInterface.h"
#include "Serialization.h"
#include <vector>
#include <thread>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>

namespace PrimeProcessor {
    class NetworkManager{
    public:
        NetworkManager(const ServerInterface& server);
        ~NetworkManager();

        void start();
        void stop();

    private:
        const ServerInterface& server;
        WSAData wsaData;
        HANDLE iocp;
        const int maxThreads = 4;
        std::vector<std::thread> workers;
        DWORD threadId = 0;

        void workerThread();
        void handleClientMessage();

        void listenClientConnections();
    };
}

#endif
