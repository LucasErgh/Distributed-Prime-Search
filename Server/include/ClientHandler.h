#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <vector>
#include <array>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <functional>

namespace PrimeProcessor{

    class ClientHandler{
    private:
        std::function<std::array<unsigned long long, 2>()> requestWorkCallback;
        std::function<void(std::vector<unsigned long long>, std::array<unsigned long long, 2>)> foundPrimesCallback;
        std::function<void(std::array<unsigned long long, 2>)> searchFailedCallback;
        std::function<void()> clientDisconnectedCallback;

        SOCKET clientSocket;

        uint8_t header[3];
        int iSendResult;
        int iResult;
        std::vector<std::byte> lastSent;
        std::array<unsigned long long, 2> lastRange;
        std::atomic_bool currentlyRunning = true;

    public:
        ClientHandler(
            SOCKET& s,
            std::function<std::array<unsigned long long, 2>()> requestWorkCallback,
            std::function<void(std::vector<unsigned long long>, std::array<unsigned long long, 2>)> foundPrimesCallback,
            std::function<void(std::array<unsigned long long, 2>)> searchFailedCallback,
            std::function<void()> clientDisconnectedCallback
        );

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
