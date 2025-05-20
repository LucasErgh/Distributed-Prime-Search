#include "NetworkManager.h"
#include <iostream>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <cassert>

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
        for (int i = 0; i < maxThreads; ++i){
            workers.emplace_back(&workerThread, this);
        }
    }

    void NetworkManager::start() {
        initialize();

        CreateAcceptSocket();

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
                throw(std::string("NO NO NO, GetQueuedCompletionStatus"));
            
            PerIOContext* IOContext = CONTAINING_RECORD(lpOverlapped, PerIOContext, overlapped);

            IOContext->bytesTransfered += bytes;

            switch (IOContext->operation)
            {
            case ACCEPT:
                    handleAccept(socketContext, IOContext);
                break;

            case SEND:
                    handleReceiveMessage(socketContext, IOContext);
                break;

            case RECV:
                    handleReceiveMessage(socketContext, IOContext);
                break;

            default:
                // @TODO handle error
                break;
            }
        }

        // Stop thread logic
    }

    void NetworkManager::handleAccept(PerSocketContext* socketContext, PerIOContext* IOContext) {
        auto nRet = setsockopt(
            IOContext->acceptSocket,
            SOL_SOCKET,
            SO_UPDATE_ACCEPT_CONTEXT,
            (char*)&listenSocket,
            sizeof(listenSocket)
        );

        // @TODO error handling
        if (IOContext->acceptSocket == INVALID_SOCKET)
            throw(std::string("NO NO NO"));

        PerSocketContext* newSock = new PerSocketContext(std::move(IOContext->acceptSocket));
        iocp = CreateIoCompletionPort((HANDLE)newSock->socket, iocp, (DWORD_PTR)newSock, 0);
        clients.push_back(newSock);

        newSock->context.emplace_back(new PerIOContext());

        handleSendMessage(newSock, newSock->context.back());
    }

    void NetworkManager::handleSendMessage(PerSocketContext* socketContext, PerIOContext* IOContext) {
        socketContext->lastRange = messageQueue->dequeueWork();

        PerIOContext* newIOContext = new PerIOContext();
        socketContext->context.push_back(newIOContext);
        newIOContext->operation = SEND;
        newIOContext->message = createMsg(socketContext->lastRange);
        newIOContext->bytesTransfered = newIOContext->message.size();
        newIOContext->wsaBuffer.buf = (char*)newIOContext->message.data();
        newIOContext->wsaBuffer.len = newIOContext->message.size();
        auto nRet = WSASend(
            socketContext->socket,
            &newIOContext->wsaBuffer, 1,
            NULL,
            0,
            &newIOContext->overlapped,
            NULL
        );
    }

    void NetworkManager::handleReceiveMessage(PerSocketContext* socketContext, PerIOContext* IOContext) {

        DWORD dwFlags = 0;
        PerIOContext* newIOContext = new PerIOContext();
        socketContext->context.push_back(newIOContext);

        std::cerr << "WSAGetLastError(): " << WSAGetLastError() << '\n';

        if (IOContext->bytesRead == 0) {
            newIOContext->bytesRead = 3;
            newIOContext->wsaBuffer.buf = newIOContext->header;
            newIOContext->wsaBuffer.len = sizeof(newIOContext->header);
            ZeroMemory(newIOContext->wsaBuffer.buf, newIOContext->wsaBuffer.len);
            newIOContext->operation = RECV;
            assert(newIOContext->wsaBuffer.buf == newIOContext->header);
            assert(newIOContext->wsaBuffer.len == sizeof(newIOContext->header));
            int retval = WSARecv(
                socketContext->socket,
                (LPWSABUF)&(newIOContext->wsaBuffer), 1,
                NULL,
                &dwFlags,
                &newIOContext->overlapped,
                NULL
            );

        } else if (IOContext->bytesRead == 3) {
            // read header
            int msgType = readMsg((uint8_t*)IOContext->header, IOContext->PayloadSize);

            if (msgType == CLOSE_CONNECTION) {
                // @TODO close connection
                return;
            }

            newIOContext->payload.resize(IOContext->PayloadSize);
            newIOContext->wsaBuffer.buf = (char*)IOContext->payload.data();
            newIOContext->wsaBuffer.len = IOContext->PayloadSize;
            DWORD size = sizeof(newIOContext->wsaBuffer.buf);
            WSARecv(
                socketContext->socket,
                &newIOContext->wsaBuffer, 1,
                NULL,
                &dwFlags,
                &newIOContext->overlapped,
                NULL
            );

        } else {
            // read payload & Send Message
            IOContext->payload.resize(IOContext->PayloadSize);
            memcpy(IOContext->payload.data(), IOContext->wsaBuffer.buf, sizeof(IOContext->wsaBuffer.buf));
            std::vector<unsigned long long> primes(IOContext->PayloadSize);
            std::fill(primes.begin(), primes.end(), 0);
            bool success = readMsg(IOContext->payload, IOContext->PayloadSize, primes);

            if (!success)
                throw (std::string("NO NO NO, readMSG"));

            messageQueue->enqueuePrimesFound(primes, socketContext->lastRange);

            socketContext->lastRange = messageQueue->dequeueWork();
            socketContext->lastSentMessage = createMsg(socketContext->lastRange);
            newIOContext->message = socketContext->lastSentMessage;
            newIOContext->wsaBuffer.buf = (char*)socketContext->lastSentMessage.data();
            newIOContext->wsaBuffer.len = sizeof(IOContext->wsaBuffer.buf);
            DWORD bytes = 3;
            newIOContext->operation = RECV;
            WSASend(
                socketContext->socket,
                &newIOContext->wsaBuffer, 1,
                NULL,
                0,
                &newIOContext->overlapped,
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

        PerSocketContext* context = new PerSocketContext(listenSocket);
        PerIOContext* ioContext = new PerIOContext();
        context->context.push_back(ioContext);

        ioContext->acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
        ioContext->operation = ACCEPT;

        iocp = CreateIoCompletionPort((HANDLE)listenSocket, iocp, (DWORD_PTR)context, 0);

        auto error = WSAGetLastError();
        if (iocp == nullptr && error != ERROR_IO_PENDING){
            std::cerr << "CreateIoCompletionPort failed: " << error << "\n";
        }

        ioContext->payload.resize( 2 * (sizeof(SOCKADDR_STORAGE) + 16));

        unsigned long bytesReceived;

        int nRet = listenSocketContext->fnAcceptEx(
            listenSocket, 
            ioContext->acceptSocket,
            ioContext->payload.data(),
            0,
            sizeof(sockaddr_storage) + 16,
            sizeof(sockaddr_storage) + 16,
            &bytesReceived,
            (LPOVERLAPPED) &(ioContext->overlapped)
        );
        context->context.push_back(std::move(ioContext));

        error = WSAGetLastError();
        if (nRet == 0 && error != ERROR_IO_PENDING){
            std::cerr << "Accept Ex failed: " << error << "\n";
        }
    }
}