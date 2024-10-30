// DPPServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "config.h"

#include <string>
#include <iostream>
#include <vector>

#include <thread>
#include <mutex>
#include <condition_variable> 
#include <atomic>
#include <chrono>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

// for stopping condition
std::atomic<bool> isReady = false;
std::mutex mtx;
std::condition_variable cv;

std::mutex listMtx;


// Function to wait for user to quit
void GetStop() {
    std::string input;
    
    do {
        input.clear();
        std::cout << "Enter \"Quit\" when you want to exit the program: ";
        std::cin >> input;
        std::cin.ignore(1000, '\n');
        std::cin.clear();
    } while (input != "Quit" && input != "quit");

    {
        std::lock_guard<std::mutex> lock(mtx);
        isReady = true;
    }

    cv.notify_one();
}

SOCKET createListner() {
    // Initialize Winsock
    WSADATA wsaData; // create object to hold windows sockets implementation
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed: \n" << iResult;
    }

    // Initialize socket object
    addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints)); // this is a macro in WinBase.h that fills a block of memory to zeros
    hints.ai_family = AF_INET; // specifies IPv4 address
    hints.ai_socktype = SOCK_STREAM; // specifies a stream socket
    hints.ai_protocol = IPPROTO_TCP; // specifies were using TCP protocol
    hints.ai_flags = AI_PASSIVE; // indicates we intend to use the returned socket address in a call to the bind function

    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result); // resolve the local addreess and port to be used by the server
    if (iResult != 0) {
        std::cout << "getaddrinfo failed:\n" << iResult;
        WSACleanup();
        return 1;
    }

    // Create a socket object called listensocket for the server to listen for client connections
    SOCKET ListenSocket = INVALID_SOCKET;

    // Call socket function and set it to ListenSocket, this creatse a socket that is bound toa specific transport service provider protocal (TCP in this case)
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ListenSocket == INVALID_SOCKET) {
        std::cout << "Error at socket():\n", WSAGetLastError();
        freeaddrinfo(result);
        throw(std::exception("Failed to bind"));
    }

    // Bind to a socket

// Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        throw(std::exception("Failed to bind"));
    }

    freeaddrinfo(result); // the information from getaddrinfo is no longer need once bind is called


}

SOCKET StartListening(SOCKET ListenSocket) {
    
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "Listen failed with error: " << WSAGetLastError();
        throw(std::exception("Failed to listen"));
    }
    else {
        SOCKET ClientSocket = INVALID_SOCKET;

        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            throw(std::exception("Failed to accept"));
        }
        else return ClientSocket;
    }
}

int main() {
    // Sockets needed
    SOCKET ListenSocket = createListner();
    std::vector<std::thread> socketList;
    
    // start control loop
    std::thread stopCondition(GetStop);
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::unique_lock<std::mutex> lock(mtx);
        if (cv.wait_for(lock, std::chrono::milliseconds(1)) == std::cv_status::no_timeout && isReady) {
            // If input is ready, break out of the loop
            break;
        }

    } while (!isReady);
    stopCondition.join();

    // TO-DO - close all sockets ands release winsock resources

    return 0;

    /*  control loop
     
            start stop condition loop
                waits for something to be put in console, when that happens stops program

            get work list (next n primes to be checked)
                list of primes to be found, probably broken into sets of 100 or 1000 in a queue
            
            start listening loop
                waits for clients to connect, when they do create their thread
            
            when client connects create their thread
                wait for primes to check
                send primes if any found, otherwise send "noprimes"
                continue
            
            store primes somehow
    
    */


}

