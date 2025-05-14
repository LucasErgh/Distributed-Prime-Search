#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "MessageQueue.h"
#include "Serialization.h"
#include "config.h"
#include <vector>
#include <thread>
#include <winsock2.h>
#include <stdio.h>
#include <mswsock.h>
#include <ws2tcpip.h>

namespace PrimeProcessor {

    class NetworkManager{
    public:
        NetworkManager(MessageQueue* messageQueue);
        ~NetworkManager();

        void start();
        void stop();

    private:

        struct OperationContext{
            WSAOVERLAPPED overlapped;
            SOCKET socket;
            std::vector<char> buffer;
            WSABUF wsabuf;
            int operationType;
            int totalBytes;
            int bytesSent;
        };

        struct ClientContext{
            SOCKET socket;
            std::vector<char> receiveBuffer;
            int key;
        };

        MessageQueue* messageQueue;

        WSAData wsaData;
        HANDLE iocp;
        addrinfo hints;

        const int maxThreads = 4;
        std::vector<std::thread> workers;
        DWORD threadId = 0;

        SOCKET listenSocket;
        SOCKET acceptSocket;

        void workerThread();
        void handleClientMessage();

        bool CreateListenSocket();
        bool CreateAcceptSocket();
    };
}

#endif
