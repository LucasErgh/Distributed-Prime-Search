#pragma once

#include "PrimeSearcher.h"
#include "Serialization.h"
#include "config.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <thread>
#include <mutex>
#include <atomic>

#include <memory>

class Connection{
private:
    // contains communication loop with server
    void serverComms();

    void connectToServer();

    // plan on making a list of workers to process the search on multiple threads
    PrimeSearch worker;
    SOCKET serverSocket = INVALID_SOCKET;
    static std::thread connectionThread;

    std::atomic<bool> closingConnection = false;
    
    uint8_t header[3];
    int iResult;

public:
    // initialize winsock library & create PrimSearcher[s] 
    Connection();

    // start connection with server
    void start();

    // close connection with server
    void stop();

    void closeConnection(){
        
    }
};
