
#include "MySockets.h"

#include <algorithm>
#include <iostream>
#include <iomanip>

namespace PrimeProcessor{

    int ClientHandler::nextKey = 1;

    void ClientHandler::closeConnection(){
        std::vector<std::byte> msg = createMsg();
        currentlyRunning = false;
        // we can just send the message since we are in in blocking mode be default
        iResult = send(clientSocket, reinterpret_cast<char*>(msg.data()), 3, 0);
        needsClosedByParent = true;
        manager->clientsToClose++;
        manager->closeClientCondition.notify_all();
        closesocket(clientSocket);
    }

    // returns array client was searching to ServerLogic work queue
    void SocketManager::searchFailed(std::array<unsigned long long, 2> arr){
        manager->returnRange(arr);
    }

    void ClientHandler::commsFailed(){
        std::cout << "Client connection unexpectedly closed: " << key << ".\n";
        manager->searchFailed(lastRange);
        closeConnection();
    }

    void ClientHandler::clientComs(){
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
                // To-Do
                if (!currentlyRunning) closeConnection();
                else {commsFailed();}
                return;
            }
            bytesReceived += iResult;

            // read msg header and check if client is closing connection
            msgType = readMsg(header, payloadSize);
            if (!msgType) {
                // To-Do
                std::cout << "Received close connection from client: " << key << '\n';
                closeConnection();
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
                    // To-Do
                    if (!currentlyRunning) closeConnection();
                    else commsFailed();
                }
                bytesReceived += iResult;
            }

            // deserialize payload and send list to server manager
            std::vector<unsigned long long> primes(payloadSize);
            std::fill(primes.begin(), primes.end(), 0);
            if (!readMsg(payload, payloadSize, primes)){
                // To-Do
                if (!currentlyRunning) closeConnection();
                else commsFailed();
                return;
            }

            // Prints for the sake of testing
            std::cout << "Received following primes from client #" << key << ":\n";
            for (auto i : primes) 
                std::cout << i << std::endl; 
            std::cout << "End of prime list" << std::endl;
            
            manager->foundPrimes(primes, lastRange); 

            // send client new range of primes
            lastRange = manager->getRange();
            lastSent = createMsg(lastRange);
            iSendResult = send(clientSocket, reinterpret_cast<char*>(lastSent.data()), lastSent.size(), 0);        
            if (iSendResult == SOCKET_ERROR){
                // To-Do
                if(!currentlyRunning) closeConnection();
                else commsFailed();
                return;
            }
            unsigned long long min, max;
            memcpy(&min, lastSent.data() + 3, sizeof(unsigned long long));
            memcpy(&max, lastSent.data() + 11, sizeof(unsigned long long));
            std::cout << "Sent client search range: (" << min << ", " << max << ")" << std::endl;
            
        } while (currentlyRunning);
        closeConnection();
    }

    ClientHandler::ClientHandler(SOCKET& s, SocketManager* m) : clientSocket(s), key(nextKey++), manager(m) { }
    
    Listener::Listener() {}

    void Listener::createSocket(){
        // Initialize SOCKET object
        ZeroMemory(&hints, sizeof(hints)); // WinBase.h macro fills memory to zero
        hints.ai_family = AF_INET; // IPv4 adress
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE; // indicates we use returned socket in bind call

        // Resolve address
        iResult = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
        if (iResult != 0){
            if (closingConnection) return;
            else throw std::runtime_error("Get address info failed with error: " + WSAGetLastError());
        }

        // Create listenerSocket
        listenerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (listenerSocket == INVALID_SOCKET) {
            freeaddrinfo(result);
            if (closingConnection) return;
            else throw std::runtime_error("Failed to create listener socket with error: " + WSAGetLastError());
        }
    }

    void Listener::startListening(){
        // bind listener 
        iResult = bind(listenerSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            freeaddrinfo(result);
            closesocket(listenerSocket);
            if (closingConnection) return;
            else throw std::runtime_error("Bind failed with error: " + WSAGetLastError());
        }

        // Start listening
        if (listen(listenerSocket, SOMAXCONN) == SOCKET_ERROR){
            closesocket(listenerSocket);
            if (closingConnection) return;
            else throw std::runtime_error("listen() failed with error: " + WSAGetLastError());
        }

        std::cout << "Started listening\n";

        while (true) {
            clientSocket = accept(listenerSocket, nullptr, nullptr);
            
            // handle failed connection
            if (clientSocket == INVALID_SOCKET) {
                if (closingConnection) break;
                closesocket(listenerSocket);
                throw std::runtime_error("Accept connection failed with error: " + WSAGetLastError());
            }
            
            // handle successful connection
            try { manager->addClient(clientSocket); }
            catch (const std::runtime_error& e) { throw e; }
            clientSocket = INVALID_SOCKET;
        }
    }

    void Listener::closeConnection(){
        closingConnection = true;
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

    void SocketManager::start(){
        listener.createSocket();
        listenThread = std::thread(&Listener::startListening, &listener);
        clientClosingThread = std::thread(&SocketManager::threadClosingLoop, this);
    }

    void SocketManager::stop(){
        // close worker and listener sockets 
        closingSocketManager = true;
        listener.closeConnection();
        listenThread.join();

        clientListMutex.lock();
        for (auto& cur : clientList){
            cur.first->needsClosedByParent = true;
            ++clientsToClose;
        }
        clientListMutex.unlock();

        closeClientCondition.notify_all();
        while(!clientClosingThread.joinable()){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            closeClientCondition.notify_all();
        }
        clientClosingThread.join();
    }

    void SocketManager::threadClosingLoop(){
        while (!closingSocketManager){
            std::unique_lock lock(clientCloseMutex);

            closeClientCondition.wait(lock);
            
            while(clientsToClose != 0){
                clientListMutex.lock();
                auto i = std::find_if(clientList.begin(), clientList.end(), [](const auto& pair){ return pair.first->needsClosedByParent == true; });
                if(i != clientList.end()){
                    i->first->closeConnection(); 
                    i->second.join();
                    clientList.erase(i);
                }
                clientListMutex.unlock();
                --clientsToClose;
            }
        }
    }
}