#include "NetworkManager.h"
#include <stdexcept>
#include <functional>

namespace PrimeProcessor {
    NetworkManager::NetworkManager(const ServerInterface& server) : server(server) {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
            throw std::runtime_error("WSAStartup failed with error: " + WSAGetLastError());
        }

        if ( (iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, maxThreads)) == NULL){
            throw std::runtime_error("CreateIoCompletionPort failed");
        }

        for (int i = 0; i < maxThreads; ++i){
            workers.emplace_back(std::move(std::thread(workerThread, this)));
        }
    }

    NetworkManager::~NetworkManager(){

    }

    void NetworkManager::start(){

    }

    void NetworkManager::stop(){

    }

    void NetworkManager::workerThread(){

    }

    void NetworkManager::handleClientMessage(){

    }

    void listenClientConnections(){

    }
}