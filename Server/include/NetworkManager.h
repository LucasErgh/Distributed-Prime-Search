#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "MessageQueue.h"
#include "Serialization.h"
#include "config.h"
#include <vector>
#include <list>
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
            RECVHEADER,
            RECVPAYLOAD,
            CLOSE,
            NONE
        };

        struct PerIOContext{
            WSAOVERLAPPED overlapped;
            WSABUF wsaBuffer{};
            std::vector<std::byte> message;
            unsigned long long bytesTransfered;
            char header[3];
            uint16_t PayloadSize;
            std::vector<std::byte> payload;
            SOCKET acceptSocket;
            OperationType operation;

            PerIOContext() : 
                acceptSocket(INVALID_SOCKET),
                bytesTransfered(0),
                PayloadSize(0)
            {
                ZeroMemory(&overlapped, sizeof(overlapped));
                overlapped.Internal = 0;
                overlapped.InternalHigh = 0;
                overlapped.Offset = 0;
                overlapped.OffsetHigh = 0;
                overlapped.hEvent = NULL;
                message.resize(3);
                payload.resize(0);
            }
        };

        struct PerSocketContext{
            SOCKET socket;
            LPFN_ACCEPTEX fnAcceptEx;
            std::vector<std::byte> lastSentMessage;
            std::array<unsigned long long, 2> lastRange;
            std::list<PerIOContext*> context;

            PerSocketContext(SOCKET socket = INVALID_SOCKET) : socket(socket) {

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
        std::mutex clientsMutex;

        SOCKET listenSocket;
        PerSocketContext* listenSocketContext;

        void workerThread();
        void handleSendMessage(PerSocketContext* socketContext, PerIOContext* IOContext);

        void PostRecv(SOCKET& socket, PerIOContext* IOContext, char* buffer, int bufferSize);
        void PostSend(SOCKET& socket, PerIOContext* IOContext, std::vector<std::byte>& msg);

        bool CreateListenSocket();
        void CreateAcceptSocket();
    };
}

#endif
