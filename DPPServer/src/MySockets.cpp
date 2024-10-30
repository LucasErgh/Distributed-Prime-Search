
#include "MySockets.h"
#include "config.h"

#include <iostream>

namespace MySockets{
    
    void SocketManager::ClientHandler::clientComs(){
        do{
            iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
            if (iResult > 0) {
                std::cout << "Bytes received: " << iResult << std::endl;

                // Echo the buffer back to the sender
                iSendResult = send(clientSocket, recvbuf, iResult, 0);
                if (iSendResult == SOCKET_ERROR) {
                    std::cout << "send failed: " << WSAGetLastError() << std::endl;
                    closesocket(clientSocket);
                    WSACleanup();
                    return;
                }
                std::cout << "Bytes sent: " << iSendResult << std::endl;
            }
            else if (iResult == 0)
                std::cout << "Connection closing...\n";
            else {
                std::cout << "recv failed: " << WSAGetLastError();
                closesocket(clientSocket);
                return;
            }
        } while (iResult > 0);
    }

    SocketManager::ClientHandler::ClientHandler(SOCKET& s) : clientSocket(s){
        
    }
    
    SocketManager::Listener::Listener(){
        addrinfo *result = nullptr, *ptr = nullptr, hints;

        // Initialize SOCKET object
        ZeroMemory(&hints, sizeof(hints)); // WinBase.h macro fills memory to zero
        hints.ai_family = AF_INET; // IPv4 adress
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE; // indicates we use returned socket in bind call

        // Resolve address
        int iResult = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
        if (iResult != 0){
            std::cout << "getaddrinfo failed: \n" << iResult;
            return;
        }

        // Create listenerSocket
        listenerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (listenerSocket == INVALID_SOCKET) {
            std::cout << "Error creating listener socket: \n" << WSAGetLastError();
            freeaddrinfo(result);
            return;
        }

        // bind listener 
        iResult = bind(listenerSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            std::cout << "bind failed with error: \n" << WSAGetLastError();
            freeaddrinfo(result);
            closesocket(listenerSocket);
            return;
        }

        // Start listening
        if (listen(listenerSocket, SOMAXCONN) == SOCKET_ERROR){
            std::cout << "Listen failed with error: \n" << WSAGetLastError();
            closesocket(listenerSocket);
            return;
        }

        // inform user of success
        std::cout << "Listener socket successfully created.\n";
    }

    void SocketManager::Listener::startListening(){
        clientSocket = accept(listenerSocket, nullptr, nullptr);
        
        // handle failed connection
        if (clientSocket == INVALID_SOCKET) {
            std::cout << "accept failed:\n" << WSAGetLastError();
            closesocket(listenerSocket);
            return;
        }
        
        // handle successful connection
        manager->addClient(clientSocket);
        return;
    }

    SocketManager::SocketManager(WSAData wsaData) : wsaData(wsaData), test(std::vector<int>()){
        // create listener and start listening
        listener.startListening();
    }

    SocketManager::~SocketManager(){
        // close connections
        // To-Do

        // Clean WinSock resources
        WSACleanup();
    }

    void SocketManager::addClient(SOCKET& c){
        std::vector<int> test2;
        test2.push_back(5);
        test.push_back(5);
        clientList.push_back(ClientHandler(c));
        listener = Listener();
        listener.startListening();
    }

}