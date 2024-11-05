
#include "MySockets.h"

#include <iostream>
#include <iomanip>

namespace PrimeProcessor{
    
    int SocketManager::ClientHandler::nextKey = 1;

    void SocketManager::ClientHandler::clientComs(){
        int msgType;
        uint16_t payloadSize;
        int bytesReceived;

        // Send and receive loop
        do{
            bytesReceived = 0;
            payloadSize = 0;
            // get message header
            iResult = recv(clientSocket, reinterpret_cast<char*>(header), 3, 0);
            if (iResult <= 0){
                closesocket(clientSocket);
                WSACleanup();
                throw std::runtime_error("receive failed: " + WSAGetLastError());
            }
            bytesReceived += iResult;

            // read msg header and check if client is closing connection
            msgType = readMsg(header, payloadSize);
            if (!msgType) {
                // To-Do close client connection
                return;
            } 

            // get payload size in bytes
            size_t payloadBytes = payloadSize * sizeof(unsigned long long);

            // read payload
            std::vector<std::byte> payload(payloadSize * sizeof(unsigned long long));
            bytesReceived = 0;
            if(payloadSize != 0) {
                iResult = recv(clientSocket, reinterpret_cast<char*>(payload.data()), payloadSize*sizeof(unsigned long long), 0);
                if (iResult < 0) {
                    closesocket(clientSocket);
                    WSACleanup();
                    throw std::runtime_error("deserialize failed: " + WSAGetLastError());
                }
                bytesReceived += iResult;
            }

            // deserialize payload and send list to server manager
            std::vector<unsigned long long> primes(payloadSize);
            std::fill(primes.begin(), primes.end(), 0);
            if (!readMsg(payload, payloadSize, primes)){
                // To-Do handle deseerialize error
                closesocket(clientSocket);
                WSACleanup();
                throw std::runtime_error("send failed: " + WSAGetLastError());
            }

            std::cout << "Received following primes from client #" << key << ":\n";
            for (auto i : primes) 
                std::cout << i << std::endl; 
            std::cout << "End of prime list" << std::endl;
            
            manager->foundPrimes(primes); 

            // send client new range of primes
            lastSent = manager->getRange();
            iSendResult = send(clientSocket, reinterpret_cast<char*>(lastSent.data()), lastSent.size(), 0);
            if (iSendResult == SOCKET_ERROR){
                closesocket(clientSocket);
                WSACleanup();
                throw std::runtime_error("send failed: " + WSAGetLastError());
            }
            unsigned long long min, max;
            memcpy(&min, lastSent.data() + 3, sizeof(unsigned long long));
            memcpy(&max, lastSent.data() + 11, sizeof(unsigned long long));
            std::cout << "Sent client search range: (" << min << ", " << max << ")" << std::endl;
            
        } while (stop);
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