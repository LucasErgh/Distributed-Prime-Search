#include "ConnectionHandler.h"

#include <stdexcept>
#include <iostream>
#include <iomanip>

std::thread Connection::connectionThread;

// contains communication loop with server
void Connection::serverComms(){
    int msgType;
    uint16_t payloadSize;
    int bytesReceived;
    std::array<unsigned long long, 2> range;

    do {
        // send message
        std::vector<unsigned long long> primes = worker.getPrimes();
        std::vector<std::byte> message = createMsg(primes);
        

        std::cout << "Sending server the following primes: " << std::endl;
        for (auto i : primes){
            std::cout << std::setw(4) << i << std::endl;
        }   std::cout << "End of prime list" << std::endl;

        int iSendResult = send(serverSocket, reinterpret_cast<char*>(message.data()), message.size(), 0);
        if(iSendResult == SOCKET_ERROR){
            closesocket(serverSocket);
            WSACleanup();
            throw std::runtime_error("send failed: " + WSAGetLastError());
        }

        // get message header
        iResult = recv(serverSocket, reinterpret_cast<char*>(header), 3, 0);
        if (iResult <= 0){
            closesocket(serverSocket);
            WSACleanup();
            throw std::runtime_error("receive failed: " + WSAGetLastError());
        }

         // read msg header and check if client is closing connection
            msgType = readMsg(header, payloadSize);
            if (!msgType) {
                // To-Do close client connection
                return;
            } 

            // get payload size in bytes
            size_t payloadBytes = payloadSize * sizeof(unsigned long long);

            // read payload
            std::vector<std::byte> payload(payloadBytes);
            iResult = recv(serverSocket, reinterpret_cast<char*>(payload.data()), payloadBytes, 0);
            if (iResult < 0) {
                closesocket(serverSocket);
                WSACleanup();
                throw std::runtime_error("deserialize failed: " + WSAGetLastError());
            }
            bytesReceived += iResult;

            // deserialize payload and send list to server manager
            if (!readMsg(payload, payloadSize, range)){
                // To-Do handle deseerialize error
                closesocket(serverSocket);
                WSACleanup();
                throw std::runtime_error("send failed: " + WSAGetLastError());
            }
            std::cout << "Received work range: (" << range[0] << ", " << range[1] << ")" << std::endl; 
            worker.newRange(range);
            worker.search();
        
    } while (iResult > 0); 
    std::cout << "Stopped communication loop" << std::endl;

}

// initialize winsock library & create PrimSearcher[s] 
Connection::Connection(){
    // initialize Winsock
    WSADATA wsaData; // Create object to hold windows sockets implementation
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        throw std::runtime_error("WSAStartup Failed: " + WSAGetLastError());
    }
}

void Connection::connectToServer(){
    // addrinfo to contain sockaddr structure
    addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints)); // set memory block to 0s
    hints.ai_family = AF_INET; // specify IPv4
    hints.ai_socktype = SOCK_STREAM; // specify stream socket
    hints.ai_protocol = IPPROTO_TCP; // specify we're using TCP protocal

    // Resolve the server address and port
    iResult = getaddrinfo(DEFAULT_ADDRESS, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        WSACleanup();
        throw std::runtime_error("getaddrinfo failed: " + WSAGetLastError());
    }

    // call the socket function to create a socket and then assign that value to ConnectSocket
    // Attempt to connect to the first address returned the call to getaddrinfo
    ptr = result;

    // Create a SOCKET for connecting to server
    serverSocket = socket(ptr->ai_family, ptr->ai_socktype,
        ptr->ai_protocol);

    if (serverSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();
        throw std::runtime_error("socket() failed: " + WSAGetLastError());
    }

    // Connect to server.
    iResult = connect(serverSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
        throw std::runtime_error("failed to connect to server: " + WSAGetLastError());
    }

    freeaddrinfo(result);

    if (serverSocket == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("failed to connect to server: " + WSAGetLastError());
    }
}

// start connection with server
void Connection::start(){
    connectToServer();
    Connection::connectionThread = std::thread(serverComms, this);
}
// close connection with server
void Connection::stop(){
    std::vector<std::byte> msg = createMsg();
    closingConnection = true;
    iResult = send(serverSocket, reinterpret_cast<char*>(msg.data()), 3, 0);
    closesocket(serverSocket);
    serverSocket = INVALID_SOCKET;
}