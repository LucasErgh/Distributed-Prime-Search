#include "ClientHandler.h"
#include "Serialization.h"
#include <iostream>

namespace PrimeProcessor{

    int ClientHandler::nextKey = 1;

    void ClientHandler::closeConnection(){
        std::vector<std::byte> msg = createMsg();
        currentlyRunning = false;
        // we can just send the message since we are in in blocking mode be default
        iResult = send(clientSocket, reinterpret_cast<char*>(msg.data()), 3, 0);
        needsClosedByParent = true;
        clientDisconnectedCallback();
        closesocket(clientSocket);
    }

    void ClientHandler::commsFailed(){
        std::cout << "Client connection unexpectedly closed: " << key << ".\n";
        searchFailedCallback(lastRange);
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
                if (!currentlyRunning)
                    closeConnection();
                else
                    commsFailed();
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
                    if (!currentlyRunning)
                        closeConnection();
                    else
                        commsFailed();
                }
                bytesReceived += iResult;
            }

            // deserialize payload and send list to server manager
            std::vector<unsigned long long> primes(payloadSize);
            std::fill(primes.begin(), primes.end(), 0);
            if (!readMsg(payload, payloadSize, primes)){
                // To-Do
                if (!currentlyRunning)
                    closeConnection();
                else
                    commsFailed();

                return;
            }

            // Prints for the sake of testing
            std::cout << "Received following primes from client #" << key << ":\n";
            for (auto i : primes) 
                std::cout << i << std::endl; 
            std::cout << "End of prime list" << std::endl;

            foundPrimesCallback(primes, lastRange); 

            // send client new range of primes
            lastRange = requestWorkCallback();
            lastSent = createMsg(lastRange);
            iSendResult = send(clientSocket, reinterpret_cast<char*>(lastSent.data()), lastSent.size(), 0);
            if (iSendResult == SOCKET_ERROR){
                // To-Do
                if(!currentlyRunning)
                    closeConnection();
                else
                    commsFailed();
                return;
            }
            unsigned long long min, max;
            memcpy(&min, lastSent.data() + 3, sizeof(unsigned long long));
            memcpy(&max, lastSent.data() + 11, sizeof(unsigned long long));
            std::cout << "Sent client search range: (" << min << ", " << max << ")" << std::endl;

        } while (currentlyRunning);
        closeConnection();
    }

    ClientHandler::ClientHandler(
            SOCKET& s,
            std::function<std::array<unsigned long long, 2>()> requestWorkCallback,
            std::function<void(std::vector<unsigned long long>, std::array<unsigned long long, 2>)> foundPrimesCallback,
            std::function<void(std::array<unsigned long long, 2>)> searchFailedCallback,
            std::function<void()> clientDisconnectedCallback
        )
        : clientSocket(s),
        key(nextKey++),
        requestWorkCallback(requestWorkCallback),
        foundPrimesCallback(foundPrimesCallback),
        searchFailedCallback(searchFailedCallback),
        clientDisconnectedCallback(clientDisconnectedCallback)
        {}
}