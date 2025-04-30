#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include "MessageQueue.h"
#include <stdio.h>
#include <vector>
#include <array>
#include <atomic>
#include <memory>
#include <functional>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace PrimeProcessor{

    class ClientHandler{
    private:
        MessageQueue* messageQueue;
        std::function<void()> clientDisconnectedCallback;

        SOCKET clientSocket;

        uint8_t header[3];
        int iSendResult;
        int iResult;
        std::vector<std::byte> lastSent;
        std::array<unsigned long long, 2> lastRange;
        std::atomic_bool currentlyRunning = true;

    public:
        ClientHandler(SOCKET& s, MessageQueue* messageQueue, std::function<void()> clientDisconnectedCallback);

        // cloeses conenction with client
        void closeConnection();

        // loop that communicates with clients
        void clientComs();
        void commsFailed();

        static int nextKey;
        int key;

        std::atomic<bool> needsClosedByParent = false;
    };

}

#endif
