
#include "MySockets.h"

#include <iostream>
#include <iomanip>

namespace PrimeProcessor{
    
    int SocketManager::ClientHandler::nextKey = 1;

    void SocketManager::ClientHandler::closeConnection(){
        std::vector<std::byte> msg = createMsg();
        stop = false;
        // we can just send the message since we are in in blocking mode be default
        iResult = send(clientSocket, reinterpret_cast<char*>(msg.data()), 3, 0);
        closesocket(clientSocket);
    }

    void SocketManager::removeClient(int key){
        clientListMutex.lock();
        for(auto i = clientList.begin(); i != clientList.end(); ++i){
            if (i->first->key == key){
                std::shared_ptr<ClientHandler> p = i->first;
                p->closeConnection();
                i->second.join();
                clientList.erase(i);
                clientListMutex.unlock();
                return;
            }
            std::cout << "Could not remove client, client was not in list.\n";
        }
        clientListMutex.unlock();
    }

    // returns array client was searching to ServerLogic work queue
    void SocketManager::searchFailed(std::array<unsigned long long, 2> arr){
        manager->returnRange(arr);
    }

    void SocketManager::ClientHandler::commsFailed(){
        std::cout << "Connection failed with client: " << key << ". Closing connection.\n";
        manager->searchFailed(lastRange);
        manager->removeClient(key);
    }

    // To-Do impliment better error handling that doesn't always close the connection

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
            if (iResult <= 0)
                if (!stop) commsFailed();
            bytesReceived += iResult;

            // read msg header and check if client is closing connection
            msgType = readMsg(header, payloadSize);
            if (!msgType) {
                std::cout << "Received close connection from client: " << key << '\n';
                manager->removeClient(key);
            } 

            // get payload size in bytes
            size_t payloadBytes = payloadSize * sizeof(unsigned long long);

            // read payload
            std::vector<std::byte> payload(payloadSize * sizeof(unsigned long long));
            bytesReceived = 0;
            if(payloadSize != 0) {
                iResult = recv(clientSocket, reinterpret_cast<char*>(payload.data()), payloadSize*sizeof(unsigned long long), 0);
                if (iResult < 0) {
                    if (!stop) commsFailed();
                    else manager->removeClient(key);
                }
                bytesReceived += iResult;
            }

            // deserialize payload and send list to server manager
            std::vector<unsigned long long> primes(payloadSize);
            std::fill(primes.begin(), primes.end(), 0);
            if (!readMsg(payload, payloadSize, primes)){
                if (!stop) commsFailed();
                else manager->removeClient(key);
            }

            std::cout << "Received following primes from client #" << key << ":\n";
            for (auto i : primes) 
                std::cout << i << std::endl; 
            std::cout << "End of prime list" << std::endl;
            
            manager->foundPrimes(primes); 

            // send client new range of primes
            lastRange = manager->getRange();
            lastSent = createMsg(lastRange);
            iSendResult = send(clientSocket, reinterpret_cast<char*>(lastSent.data()), lastSent.size(), 0);
            if (iSendResult == SOCKET_ERROR){
                if(!stop) commsFailed();
                else manager->removeClient(key);
            }
            unsigned long long min, max;
            memcpy(&min, lastSent.data() + 3, sizeof(unsigned long long));
            memcpy(&max, lastSent.data() + 11, sizeof(unsigned long long));
            std::cout << "Sent client search range: (" << min << ", " << max << ")" << std::endl;
            
        } while (stop);

        manager->removeClient(key);
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
            if (closed) return;
            else throw std::runtime_error("Get address info failed with error: " + WSAGetLastError());
        }

        // Create listenerSocket
        listenerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (listenerSocket == INVALID_SOCKET) {
            freeaddrinfo(result);
            if (closed) return;
            else throw std::runtime_error("Failed to create listener socket with error: " + WSAGetLastError());
        }
    }

    void SocketManager::Listener::startListening(){
        // bind listener 
        iResult = bind(listenerSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            freeaddrinfo(result);
            closesocket(listenerSocket);
            if (closed) return;
            else throw std::runtime_error("Bind failed with error: " + WSAGetLastError());
        }

        // Start listening
        if (listen(listenerSocket, SOMAXCONN) == SOCKET_ERROR){
            closesocket(listenerSocket);
            if (closed) return;
            else throw std::runtime_error("listen() failed with error: " + WSAGetLastError());
        }

        std::cout << "Started listening\n";

        while (true) {
            clientSocket = accept(listenerSocket, nullptr, nullptr);
            
            // handle failed connection
            if (clientSocket == INVALID_SOCKET) {
                closesocket(listenerSocket);
                if (closed) return;
                else throw std::runtime_error("Accept connection failed with error: " + WSAGetLastError());
            }
            
            // handle successful connection
            try { manager->addClient(clientSocket); }
            catch (const std::runtime_error& e) { throw e; }
        }
    }

    void SocketManager::Listener::closeConnection(){
        closed = true;
        closesocket(listenerSocket);
        if(clientSocket != INVALID_SOCKET) closesocket(clientSocket);
        return;
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
        // close worker and listener sockets 
        clientListMutex.lock();
        for(auto i = clientList.begin(); i != clientList.end(); ++i){
            i->first->closeConnection();
            i->second.join();
            clientList.erase(i);
        }
        clientListMutex.unlock();
        listener.closeConnection();

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