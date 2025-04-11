
#include "MySockets.h"

#include <algorithm>
#include <iostream>
#include <iomanip>

namespace PrimeProcessor{

    // returns array client was searching to ServerLogic work queue
    void SocketManager::searchFailed(std::array<unsigned long long, 2> arr){
        server.workFailed(arr);
    }

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

    SocketManager::SocketManager(ServerInterface& s) : server(s){
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
