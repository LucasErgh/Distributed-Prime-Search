#include "NetworkManager.h"
#include <iostream>
#include <stdexcept>
#include <functional>

namespace PrimeProcessor {
    struct perSocketContext{

    };

    NetworkManager::NetworkManager(MessageQueue* messageQueue) : messageQueue(messageQueue) {

    }

    NetworkManager::~NetworkManager() {

    }

    void NetworkManager::initialize() {

        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
            throw std::runtime_error("WSAStartup failed with error: " + WSAGetLastError());
        }

        if (!CreateListenSocket()){
            throw std::runtime_error("Failed to create listener socket\n");
        }

        if ( (iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL ){
            throw std::runtime_error("CreateIoCompletionPort failed");
        }
        CreateIoCompletionPort((HANDLE)listenSocket, iocp, (ULONG_PTR)listenSocket, 0);

        for (int i = 0; i < maxThreads; ++i){
            workers.emplace_back(&workerThread, this);
        }
    }

    void NetworkManager::start() {
        initialize();

        CreateListenSocket();

        LPDWORD bytes;
        SOCKET acceptSocket;
        // AcceptEx(listenSocket, acceptSocket)
    }

    void NetworkManager::stop(){

    }

    void NetworkManager::workerThread(){
        LPWSAOVERLAPPED lpOverlapped = nullptr;
        PerSocketContext* socketContext;
        unsigned long bytes = 0;

        while (true /* @TODO later add an atomic for running */) {
            bool success = GetQueuedCompletionStatus(
                iocp,
                &bytes,
                (PDWORD_PTR)&socketContext,
                (LPOVERLAPPED*)&lpOverlapped,
                INFINITE
            );

            if (!success)
                throw("NO NO NO");
            
            socketContext->context->bytesTransfered += bytes;

            switch (socketContext->context->operation)
            {
            case ACCEPT:
                    handleAccept(socketContext);
                break;

            case SEND:
                    socketContext->context = std::make_unique<PerIOContext>();
                    handleReceiveMessage(socketContext);
                break;

            case RECV:
                    handleReceiveMessage(socketContext);
                break;

            default:
                // @TODO handle error
                break;
            }
        }

        // Stop thread logic
    }

    void NetworkManager::handleAccept(PerSocketContext* socketContext) {
        auto nRet = setsockopt(
            socketContext->context->acceptSocket,
            SOL_SOCKET,
            SO_UPDATE_ACCEPT_CONTEXT,
            (char*)&listenSocket,
            sizeof(listenSocket)
        );

        // @TODO error handling
        if (socketContext->context->acceptSocket == INVALID_SOCKET)
            throw("NO NO NO");

        PerSocketContext* newSock = new PerSocketContext(socketContext->context->acceptSocket);
        iocp = CreateIoCompletionPort((HANDLE)newSock->socket, iocp, (DWORD_PTR)newSock, 0);
        clients.push_back(newSock);

        handleReceiveMessage(socketContext);
    }

    void NetworkManager::handleSendMessage(PerSocketContext* socketContext) {
        socketContext->lastRange = messageQueue->dequeueWork();
        socketContext->context->message = createMsg(socketContext->lastRange);
        socketContext->context->bytesTransfered = socketContext->context->message.size();
        socketContext->context->wsaBuffer.buf = (char*)socketContext->context->message.data();
        socketContext->context->wsaBuffer.len = socketContext->context->message.size();
        auto nRet = WSASend(
            socketContext->socket,
            &socketContext->context->wsaBuffer, 1,
            (LPDWORD)&socketContext->context->bytesTransfered,
            0,
            &socketContext->context->overlapped,
            NULL
        );
    }

    void NetworkManager::handleReceiveMessage(PerSocketContext* socketContext) {
        // @TODO
        if (socketContext->context->bytesRead == 0) {
            // read header
            int msgType = readMsg((uint8_t*)socketContext->context->header, socketContext->context->PayloadSize);
            socketContext->context->bytesRead = 3;

            if (msgType == CLOSE_CONNECTION) {
                // @TODO close connection
                return;
            }

            socketContext->context->payload.resize(socketContext->context->PayloadSize);
            socketContext->context->wsaBuffer.buf = (char*)(socketContext->context->payload.data());
            socketContext->context->wsaBuffer.len = socketContext->context->PayloadSize;
            DWORD size = socketContext->context->PayloadSize;
            WSARecv(
                socketContext->socket,
                &socketContext->context->wsaBuffer, 1,
                &size,
                0,
                &socketContext->context->overlapped,
                NULL
            );

        } else {
            // read payload & Send Message
            memcpy(socketContext->context->payload.data(), socketContext->context->wsaBuffer.buf, sizeof(socketContext->context->wsaBuffer.buf));
            std::vector<unsigned long long> primes(socketContext->context->PayloadSize);
            std::fill(primes.begin(), primes.end(), 0);
            bool success = readMsg(socketContext->context->payload, socketContext->context->PayloadSize, primes);

            if (!success)
                throw ("NO NO NO");

            messageQueue->enqueuePrimesFound(primes, socketContext->lastRange);

            socketContext->context = std::make_unique<PerIOContext>();
            socketContext->lastRange = messageQueue->dequeueWork();
            socketContext->lastSentMessage = createMsg(socketContext->lastRange);
            socketContext->context->message = socketContext->lastSentMessage;
            socketContext->context->wsaBuffer.buf = (char*)socketContext->lastSentMessage.data();
            socketContext->context->wsaBuffer.len = sizeof(socketContext->context->wsaBuffer.buf);
            DWORD bytes = 3;
            WSASend(
                socketContext->socket,
                &socketContext->context->wsaBuffer, 1,
                &bytes,
                0,
                &socketContext->context->overlapped,
                NULL
            );
        }
    }

    bool NetworkManager::CreateListenSocket() {
        int nRet = 0;
        int nZero = 0;
        addrinfo hints = {0};
        addrinfo *addrlocal = NULL;

        hints.ai_flags  = AI_PASSIVE;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_IP;
        hints.ai_flags = AI_PASSIVE;

        if( getaddrinfo(nullptr, DEFAULT_PORT, &hints, &addrlocal) != 0 ) {
            std::cerr << "getaddrinfo() failed with error" << WSAGetLastError() << '\n';
            return(FALSE);
        }

        if( addrlocal == NULL ) {
            std::cerr << "getaddrinfo() failed to resolve/convert the interface\n";
            return(FALSE);
        }

        listenSocket = WSASocket(
            addrlocal->ai_family, addrlocal->ai_socktype, addrlocal->ai_protocol,
            NULL, 0, WSA_FLAG_OVERLAPPED
        );

        // Disable send buffering on the socket.
        nZero = 0;
        nRet = setsockopt(listenSocket, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));
        if( nRet == SOCKET_ERROR ) {
            std::cerr << "setsockopt(SNDBUF) failed: " << WSAGetLastError() << '\n';
            return(FALSE);
        }

        if( listenSocket == INVALID_SOCKET ) {
            std::cerr << "WSASocket(listenSocket) failed: " << WSAGetLastError() << '\n';
            return(FALSE);
        }

        nRet = bind(listenSocket, addrlocal->ai_addr, (int) addrlocal->ai_addrlen);
        if( nRet == SOCKET_ERROR ) {
            std::cerr << "bind() failed: " << WSAGetLastError() << '\n';
            return(FALSE);
        }

        nRet = listen(listenSocket, 5);
        if( nRet == SOCKET_ERROR ) {
            std::cerr << "listen() failed: " << WSAGetLastError() << '\n';
            return(FALSE);
        }


        GUID acceptexID = WSAID_ACCEPTEX;
        DWORD bytes = 0;

        listenSocketContext = new PerSocketContext(listenSocket);

        // Load AcceptEx extension
        nRet = WSAIoctl(
            listenSocket,
            SIO_GET_EXTENSION_FUNCTION_POINTER,
            &acceptexID, sizeof(acceptexID),
            &listenSocketContext->fnAcceptEx, sizeof(listenSocketContext->fnAcceptEx),
            &bytes,
            NULL,
            NULL
        );

        return true;
    }

    void NetworkManager::CreateAcceptSocket(){

        // @TODO handle error

        PerSocketContext* context = new PerSocketContext();
        context->context->acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
        context->context->operation = ACCEPT;

        iocp = CreateIoCompletionPort((HANDLE)listenSocket, iocp, (DWORD_PTR)context, 0);

        if (iocp == nullptr)
            throw("NO NO NO");

        context->socket = listenSocket;
        context->context->acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

        unsigned long bytesReceived;

        int nRet = listenSocketContext->fnAcceptEx(
            listenSocket, 
            context->context->acceptSocket,
            context->context->header,
            sizeof(context->context->header),
            sizeof(sockaddr_storage) + 16,
            sizeof(sockaddr_storage) + 16,
            &bytesReceived,
            (LPOVERLAPPED) &(context->context->overlapped)
        );
    }
}