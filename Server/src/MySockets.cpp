#include "MySockets.h"

#include <algorithm>
#include <iostream>
#include <iomanip>

namespace PrimeProcessor{

    // returns array client was searching to ServerLogic work queue
    void SocketManager::searchFailed(std::array<unsigned long long, 2> arr){
        server.workFailed(arr);
    }

    SocketManager::SocketManager(ServerInterface& s) : server(s){
        // Initialize WinSock
        WSAData wsaData;
        int iResult;
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            throw std::runtime_error("WSAStartup failed with error: " + WSAGetLastError());
        }

        listener = std::make_unique<Listener>(std::bind(&addClient, this, std::placeholders::_1));
    }

    SocketManager::~SocketManager(){
        // Clean WinSock resources
        WSACleanup();
    }

    void SocketManager::addClient(SOCKET& c){
        clientListMutex.lock();
        std::shared_ptr<ClientHandler> client = std::make_shared<ClientHandler>(
            c,
            std::bind(&requestWork, this),
            std::bind(&foundPrimes, this, std::placeholders::_1, std::placeholders::_2),
            std::bind(&searchFailed, this, std::placeholders::_1),
            std::bind(&clientDisconnected, this)
        );
        std::thread cThread(ClientHandler::clientComs, client);
        clientList.emplace_back(std::move(client), std::move(cThread));
        clientListMutex.unlock();
    }

    void SocketManager::start(){
        listener->createSocket();
        listenThread = std::thread(&Listener::startListening, &(*listener));
        clientClosingThread = std::thread(&SocketManager::threadClosingLoop, this);
    }

    void SocketManager::stop(){
        // close worker and listener sockets 
        closingSocketManager = true;
        listener->closeConnection();
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


    void SocketManager::clientDisconnected(){
        clientsToClose++;
        closeClientCondition.notify_all();
    }
}
