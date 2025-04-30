#include "NetworkManager.h"
#include <iostream>
#include <stdexcept>
#include <functional>

namespace PrimeProcessor {
    struct perSocketContext{

    };

    NetworkManager::NetworkManager(MessageQueue* messageQueue) : messageQueue(messageQueue) {

    }

    NetworkManager::~NetworkManager(){

    }

    void NetworkManager::start(){

        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
            throw std::runtime_error("WSAStartup failed with error: " + WSAGetLastError());
        }

        if ( (iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, maxThreads)) == NULL ){
            throw std::runtime_error("CreateIoCompletionPort failed");
        }

        for (int i = 0; i < maxThreads; ++i){
            workers.emplace_back(std::thread(workerThread, this));
        }

        if (!CreateListenSocket()){
            throw std::runtime_error("Failed to create listener socket\n");
        }

        // Loop accepting connections
        while (true){
            acceptSocket = WSAAccept(listenSocket, NULL, NULL, NULL, 0);
            if (acceptSocket == SOCKET_ERROR){
                std::cerr << "WSAAccept() failed: " << WSAGetLastError() << '\n';
            }

            // add socket descriptor to the IOCP along with its accociated key data


            // post initial receive on this socket
        }
    }

    void NetworkManager::stop(){

    }

    void NetworkManager::workerThread(){

    }

    void NetworkManager::handleClientMessage(){

    }

    bool NetworkManager::CreateListenSocket(){
        int nRet = 0;
        int nZero = 0;
        addrinfo hints = {0};
        addrinfo *addrlocal = NULL;

        hints.ai_flags  = AI_PASSIVE;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_IP;

        if( getaddrinfo(NULL, DEFAULT_PORT, &hints, &addrlocal) != 0 ) {
            std::cerr << "getaddrinfo() failed with error" << WSAGetLastError() << '\n';
            return(FALSE);
        }

        if( addrlocal == NULL ) {
            std::cerr << "getaddrinfo() failed to resolve/convert the interface\n";
            return(FALSE);
        }

        listenSocket = WSASocket(addrlocal->ai_family, addrlocal->ai_socktype, addrlocal->ai_protocol, 
                            NULL, 0, WSA_FLAG_OVERLAPPED); 
        if( listenSocket == INVALID_SOCKET ) {
            std::cerr << "WSASocket(listenSocket) failed:" << WSAGetLastError() << '\n';
            return(FALSE);
        }

        nRet = bind(listenSocket, addrlocal->ai_addr, (int) addrlocal->ai_addrlen);
        if( nRet == SOCKET_ERROR ) {
            std::cerr << "bind() failed:" << WSAGetLastError() << '\n';
            return(FALSE);
        }

        nRet = listen(listenSocket, 5);
        if( nRet == SOCKET_ERROR ) {
            std::cerr << "listen() failed:" << WSAGetLastError() << '\n';
            return(FALSE);
        }

        // Disable send buffering on the socket.
        nZero = 0;
        nRet = setsockopt(listenSocket, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));
        if( nRet == SOCKET_ERROR ) {
            std::cerr << "setsockopt(SNDBUF) failed: " << WSAGetLastError() << '\n';
            return(FALSE);
        }
    }

    void listenClientConnections(){

    }
}