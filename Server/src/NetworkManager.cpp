#include "NetworkManager.h"
#include <iostream>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <cassert>

namespace PrimeProcessor {

    NetworkManager::NetworkManager(MessageQueue* messageQueue) : messageQueue(messageQueue) {

    }

    NetworkManager::~NetworkManager() {
        if (running)
            stop();
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
        std::cout << "Starting Server\n";
        initialize();
        CreateAcceptSocket();
    }

    void NetworkManager::stop(){

        {
            std::unique_lock<std::mutex> lock(clientsMutex);
            for (auto cur : clients) {
                PostClose(cur);
            }

            if (!clients.empty())
                clientConditional.wait(lock, [this](){ return clients.empty(); });
        }

        running = false;
        for (int i = 0; i < workers.size(); ++i){
            PostNull();
        }
        for (auto& cur : workers) {
            cur.join();
        }

        closesocket(listenSocket);
        if (listenSocketContext)
            delete listenSocketContext;

        CloseHandle(iocp);
        WSACleanup();
    }

    void NetworkManager::workerThread(){
        LPWSAOVERLAPPED lpOverlapped = nullptr;
        PerSocketContext* socketContext;
        unsigned long bytes = 0;

        while (running) {
            bool success = GetQueuedCompletionStatus(
                iocp,
                &bytes,
                (PDWORD_PTR)&socketContext,
                (LPOVERLAPPED*)&lpOverlapped,
                INFINITE
            );

            PerIOContext* IOContext = CONTAINING_RECORD(lpOverlapped, PerIOContext, overlapped);

            // checks for key given by PostNull to stop waiting for GetQueuedCompletionStatus and continue
            if (socketContext == nullptr) {
                delete IOContext;
                continue;
            }

            if (!success && std::find(clients.begin(), clients.end(), socketContext) == clients.end()){
                delete IOContext;
                continue;
            }

            if (!success) {
                std::cout << "Client disconnected unexpectedly\n";
                {
                    std::unique_lock<std::mutex> lock(clientsMutex);
                    messageQueue->searchFailed(socketContext->lastRange);
                    closesocket(socketContext->socket);
                    socketContext->socket = INVALID_SOCKET;
                    auto it = std::find(clients.begin(), clients.end(), socketContext);
                    delete *it;
                    clients.erase(it);
                    continue;
                }
                // std::cerr << "GetQueuedCompletionStatus failed with last error: " << GetLastError() << '\n';
            }

            IOContext->bytesTransfered += bytes;

            switch (IOContext->operation)
            {
            case ACCEPT:
                {
                    auto nRet = setsockopt(
                        IOContext->acceptSocket,
                        SOL_SOCKET,
                        SO_UPDATE_ACCEPT_CONTEXT,
                        (char*)&listenSocket,
                        sizeof(listenSocket)
                    );
                    // @TODO error handling
                    if (IOContext->acceptSocket == INVALID_SOCKET){
                        std::cerr << "Worker thread failed in case ACCEPT";
                        throw(std::string("NO NO NO"));
                    }
                    PerSocketContext* newSock = new PerSocketContext(std::move(IOContext->acceptSocket));
                    iocp = CreateIoCompletionPort((HANDLE)newSock->socket, iocp, (DWORD_PTR)newSock, 0);
                    {
                        std::unique_lock<std::mutex> lock(clientsMutex);
                        clients.push_back(newSock);
                    }
                    std::cout << "Client connected\n";
                    socketContext->removeContext(IOContext);
                    CreateAcceptSocket();
                    handleSendMessage(newSock, IOContext);
                    break;
                }
            case SEND:
                {
                    PerIOContext* newIOContext = new PerIOContext();
                    socketContext->context.push_back(newIOContext);
                    newIOContext->operation = RECVHEADER;
                    socketContext->removeContext(IOContext);
                    PostRecv(socketContext->socket, newIOContext, newIOContext->header, sizeof(newIOContext->header));
                    break;
                }
            case RECVHEADER:
                {
                    int msgType = readMsg((uint8_t*)IOContext->header, IOContext->PayloadSize);
                    if (msgType == CLOSE_CONNECTION) {
                        std::cerr << "Close Connection Sent From Client\n\n";
                        {
                            std::unique_lock<std::mutex> lock(clientsMutex);
                            messageQueue->searchFailed(socketContext->lastRange);
                            auto it = std::find(clients.begin(), clients.end(), socketContext);
                            clients.erase(it);
                            clientConditional.notify_one();
                            socketContext->removeContext(IOContext);
                            delete socketContext;
                        }
                        continue;
                    }
                    IOContext->operation = RECVPAYLOAD;
                    IOContext->payload.resize(IOContext->PayloadSize * sizeof(unsigned long long));
                    PostRecv(socketContext->socket, IOContext, (char*)IOContext->payload.data(), (IOContext->PayloadSize * sizeof(unsigned long long)));
                    break;
                }
            case RECVPAYLOAD:
                {
                    std::vector<unsigned long long> primes(IOContext->PayloadSize);
                    std::fill(primes.begin(), primes.end(), 0);
                    bool success = readMsg(IOContext->payload, IOContext->PayloadSize, primes);
                    if (!success){
                        std::cerr << "Worker thread failed in case RECVPAYLOAD";
                        throw (std::string("NO NO NO, readMSG"));
                    }
                    messageQueue->enqueuePrimesFound(primes, socketContext->lastRange);
                    socketContext->lastRange.fill(0);
                    socketContext->removeContext(IOContext);
                    handleSendMessage(socketContext, IOContext);
                    break;
                }
            case CLOSE:
                {
                    std::unique_lock<std::mutex> lock(clientsMutex);
                    messageQueue->searchFailed(socketContext->lastRange);
                    closesocket(socketContext->socket);
                    socketContext->socket = INVALID_SOCKET;
                    auto it = std::find(clients.begin(), clients.end(), socketContext);
                    clients.erase(it);
                    delete *it;
                    clientConditional.notify_one();
                    break;
                }
            default:
                // @TODO handle error
                break;
            }
        }
    }

    void NetworkManager::handleSendMessage(PerSocketContext* socketContext, PerIOContext* IOContext) {
        PerIOContext* newIOContext = new PerIOContext();
        socketContext->context.push_back(newIOContext);
        socketContext->lastRange = messageQueue->dequeueWork();
        socketContext->lastSentMessage = createMsg(socketContext->lastRange);
        PostSend(socketContext->socket, newIOContext, socketContext->lastSentMessage);
    }

    void NetworkManager::PostRecv(SOCKET& socket, PerIOContext* IOContext, char* buffer, int bufferSize) {
        DWORD dwFlags = 0;
        //ZeroMemory(buffer, bufferSize);
        IOContext->wsaBuffer.buf = buffer;
        IOContext->wsaBuffer.len = bufferSize;
        int retval = WSARecv(
            socket,
            (LPWSABUF)&(IOContext->wsaBuffer), 1,
            NULL,
            &dwFlags,
            &IOContext->overlapped,
            NULL
        );
    }

    void NetworkManager::PostSend(SOCKET& socket, PerIOContext* IOContext, std::vector<std::byte>& msg, OperationType operation) {
        IOContext->message = msg;
        IOContext->wsaBuffer.buf = (char*)IOContext->message.data();
        IOContext->wsaBuffer.len = IOContext->message.size();
        IOContext->operation = operation;
        WSASend(
            socket,
            &IOContext->wsaBuffer, 1,
            NULL,
            0,
            &IOContext->overlapped,
            NULL
        );
    }

    void NetworkManager::PostNull(){
        PerIOContext* IOContext = new PerIOContext();
        PostQueuedCompletionStatus(iocp, 0, 0, &IOContext->overlapped);
    }

    void NetworkManager::PostClose(PerSocketContext* socketContext) {
        PerIOContext* newIOContext = new PerIOContext();
        socketContext->context.push_back(newIOContext);
        socketContext->lastSentMessage = createMsg();
        PostSend(socketContext->socket, newIOContext, socketContext->lastSentMessage, CLOSE);
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
        static bool needUpdateIOCP = true;
        // @TODO handle error

        PerIOContext* ioContext = new PerIOContext();
        listenSocketContext->context.push_back(ioContext);

        ioContext->acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
        ioContext->operation = ACCEPT;

        if (needUpdateIOCP) {
            iocp = CreateIoCompletionPort((HANDLE)listenSocket, iocp, (DWORD_PTR)listenSocketContext, 0);
            needUpdateIOCP = false;
        }

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

        error = WSAGetLastError();
        if (nRet == 0 && error != ERROR_IO_PENDING){
            std::cerr << "Accept Ex failed: " << error << "\n";
        }
    }
}