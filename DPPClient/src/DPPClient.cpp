// DPPClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "config.h"

#include <iostream>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

int main()
{
    /*------------------------------------------------------------------------------------------------------------------------------------*/
    // initialize Winsock
    WSADATA wsaData; // Create object to hold windows sockets implementation
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed: \n" << iResult;
        return 1;
    }

    /*------------------------------------------------------------------------------------------------------------------------------------*/
    // Create a socket for the client

    // addrinfo to contain sockaddr structure
    addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints)); // set memory block to 0s
    hints.ai_family = AF_INET; // specify IPv4
    hints.ai_socktype = SOCK_STREAM; // specify stream socket
    hints.ai_protocol = IPPROTO_TCP; // specify we're using TCP protocal

    // Resolve the server address and port
    iResult = getaddrinfo(DEFAULT_ADDRESS, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a socket object now
    SOCKET ConnectSocket = INVALID_SOCKET;

    // call the socket function to create a socket and then assign that value to ConnectSocket
    // Attempt to connect to the first address returned the call to getaddrinfo
    ptr = result;

    // Create a SOCKET for connecting to server
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
        ptr->ai_protocol);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    /*------------------------------------------------------------------------------------------------------------------------------------*/
    // Connect to a socket

    // Connect to server.
    iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    // Should really try the next address returned by getaddrinfo
    // if the connect call failed
    // But for this simple example we just free the resources
    // returned by getaddrinfo and print an error message

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    /*------------------------------------------------------------------------------------------------------------------------------------*/
    // Send and receive data

    const char* sendbuf = "this is a test";
    char recvbuf[DEFAULT_BUFLEN];

    iResult;

    // Send an initial buffer
    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        std::cout << "send failed: " << WSAGetLastError() << std::endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Bytes Sent: " << iResult << std::endl;

    // Shutdown the connection for sending since no more data will be sent
    // Client can still use the connectsocket for receiving data
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        std::cout << "shutdown failed: " << WSAGetLastError() << std::endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive data until the server closes the connection
    do {
        iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0)
            std::cout << "Bytes received: " << iResult << std::endl;
        else if (iResult == 0)
            std::cout << "connection closed\n";
        else
            std::cout << "recv failed: " << WSAGetLastError() << std::endl;

    } while (iResult > 0);

    /*------------------------------------------------------------------------------------------------------------------------------------*/
    //Disconnecting the client

    // shutdown the send half of the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        std::cout << "shutdown failed: " << WSAGetLastError << std::endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    closesocket(ConnectSocket); // close socket
    WSACleanup(); // release windows socket resources

    /*------------------------------------------------------------------------------------------------------------------------------------*/

    return 0;
}

