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
    private:
        enum OperationType{
            ACCEPT,
            SEND,
            RECV
        };

        struct PerIOContext{
            WSAOVERLAPPED overlapped;
            WSABUF wsaBuffer;
            std::vector<std::byte> message;
            unsigned long long bytesTransfered;
            char header[3];
            uint16_t PayloadSize;
            std::vector<std::byte> payload;
            int bytesRead;
            SOCKET acceptSocket;
            OperationType operation;

            PerIOContext() : acceptSocket(INVALID_SOCKET) {
                overlapped.Internal = 0;
                overlapped.InternalHigh = 0;
                overlapped.Offset = 0;
                overlapped.OffsetHigh = 0;
                overlapped.hEvent = NULL;
            }
        };

        struct PerSocketContext{
            SOCKET socket;
            LPFN_ACCEPTEX fnAcceptEx;
            std::vector<std::byte> lastSentMessage;
            std::array<unsigned long long, 2> lastRange;
            std::unique_ptr<PerIOContext> context;

            PerSocketContext(SOCKET socket = INVALID_SOCKET) : socket(socket) {
                context = std::make_unique<PerIOContext>();
            }
        };

    public:
        NetworkManager(MessageQueue* messageQueue);
        ~NetworkManager();

        void initialize();
        void start();
        void stop();

    private:
        MessageQueue* messageQueue;

        WSAData wsaData;
        HANDLE iocp;
        addrinfo hints;

        const int maxThreads = 4;
        std::vector<std::thread> workers;
        DWORD threadId = 0;

        std::vector<PerSocketContext*> clients;
        std::mutex clientMutex;

        SOCKET listenSocket;
        PerSocketContext* listenSocketContext;

        void workerThread();
        void handleSendMessage(PerSocketContext* socketContext);
        void handleReceiveMessage(PerSocketContext* socketContext);
        void handleAccept(PerSocketContext* socketContext);

        bool CreateListenSocket();
        void CreateAcceptSocket();
    };
}

#endif
