
#include "MySockets.h"

#include <iostream>

namespace PrimeProcessor{
    
    int SocketManager::ClientHandler::nextKey = 1;

    void SocketManager::ClientHandler::clientComs(){
        int msgType;
        uint16_t payloadSize;
        int bytesReceived = 0;

        // Send and receive loop for testing
        do{
            // get message header
            uint8_t header[3];
            for (int i = 0; i < 3; i++) {
                iResult = recv(clientSocket, reinterpret_cast<char*>(header) + i, 2 - bytesReceived, 0);
                if (iResult <= 0){
                    // To-Do handle recv error
                }
                bytesReceived += iResult;
            }

            // Check if client is closing connection
            msgType = header[0];
            if (msgType == 0) {
                // To-Do close client connection
                return;
            }

            // get payload size 
            payloadSize = header[1];
            size_t payloadBytes = payloadSize * sizeof(ull);

            // read payload
            std::vector<std::byte> payload(payloadBytes);
            bytesReceived = 0;
            while(bytesReceived < payloadBytes) {
                iResult = recv(clientSocket, reinterpret_cast<char*>(payload.data()) + bytesReceived, payloadBytes - bytesReceived, 0);
                if (iResult < 0) {
                    // To-Do handle recv error
                }
                bytesReceived += iResult;
            }

            // send primes found to server

            /* test code 
            iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
            if (iResult > 0) {
                std::this_thread::sleep_for(std::chrono::seconds(3));

                std::cout << "Bytes received: " << iResult << std::endl;

                iSendResult = send(clientSocket, recvbuf, iResult, 0);

                if (iSendResult == SOCKET_ERROR) {
                    closesocket(clientSocket);
                    WSACleanup();
                    throw std::runtime_error("send failed: " + WSAGetLastError());
                }
                std::cout << "Bytes sent: " << iSendResult << std::endl;
            }
            else if (iResult == 0)
                std::cout << "Connection closing...\n";
            else {
                closesocket(clientSocket);
                throw std::runtime_error("recv failed: " + WSAGetLastError());
            }
            */
        } while (iResult > 0);
    }

    SocketManager::ClientHandler::ClientHandler(SOCKET& s, SocketManager* m) : clientSocket(s), key(nextKey++), manager(m) { }
    
    SocketManager::Listener::Listener(){}

    void SocketManager::Listener::createSocket(){
        // Initialize SOCKET object
        ZeroMemory(&hints, sizeof(hints)); // WinBase.h macro fills memory to zero
        hints.ai_family = AF_INET; // IPv4 adress
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE; // indicates we use returned socket in bind call

        // Resolve address
        iResult = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
        if (iResult != 0){
            throw std::runtime_error("Get address info failed with error: " + WSAGetLastError());
        }

        // Create listenerSocket
        listenerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (listenerSocket == INVALID_SOCKET) {
            freeaddrinfo(result);
            throw std::runtime_error("Failed to create listener socket with error: " + WSAGetLastError());
        }
    }

    void SocketManager::Listener::startListening(){
        // bind listener 
        iResult = bind(listenerSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            freeaddrinfo(result);
            closesocket(listenerSocket);
            throw std::runtime_error("Bind failed with error: " + WSAGetLastError());
        }

        // Start listening
        if (listen(listenerSocket, SOMAXCONN) == SOCKET_ERROR){
            closesocket(listenerSocket);
            throw std::runtime_error("Bind failed with error: " + WSAGetLastError());
        }

        std::cout << "Started listening\n";

        while (true) {
            clientSocket = accept(listenerSocket, nullptr, nullptr);
            
            // handle failed connection
            if (clientSocket == INVALID_SOCKET) {
                closesocket(listenerSocket);
                throw std::runtime_error("Accept connection failed with error: " + WSAGetLastError());
            }
            
            // handle successful connection
            try { manager->addClient(clientSocket); }
            catch (const std::runtime_error& e) { throw e; }
        }
    }

    SocketManager::SocketManager(ServerLogic* s) : manager(s){
        // Initialize WinSock
        WSAData wsaData;
        int iResult;
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            throw std::runtime_error("WSAStartup failed with error: " + WSAGetLastError());
        }

        // set listener parent to this
        listener.manager = this;
    }

    SocketManager::~SocketManager(){
        // close connections
        // To-Do

        // Clean WinSock resources
        WSACleanup();
    }

    void SocketManager::addClient(SOCKET& c){

        clientListMutex.lock();
        std::shared_ptr<ClientHandler> client = std::make_shared<ClientHandler>(c, this);
        std::thread cThread(ClientHandler::clientComs, client);
        clientList.emplace_back(std::move(client), std::move(cThread));
        clientListMutex.unlock();
    }

}