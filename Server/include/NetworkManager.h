#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "ServerInterface.h"
#include "Serialization.h"
#include "config.h"
#include <vector>
#include <thread>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>

namespace PrimeProcessor {
    typedef enum _IO_OPERATION {
        ClientIoAccept,
        ClientIoRead,
        ClientIoWrite
    } IO_OPERATION, *PIO_OPERATION;

    //
    // data to be associated for every I/O operation on a socket
    //
    typedef struct _PER_IO_CONTEXT {
        WSAOVERLAPPED               Overlapped;
        char                        Buffer[DEFAULT_BUFLEN];
        WSABUF                      wsabuf;
        int                         nTotalBytes;
        int                         nSentBytes;
        IO_OPERATION                IOOperation;
        SOCKET                      SocketAccept; 

        struct _PER_IO_CONTEXT      *pIOContextForward;
    } PER_IO_CONTEXT, *PPER_IO_CONTEXT;

    //
    // For AcceptEx, the IOCP key is the PER_SOCKET_CONTEXT for the listening socket,
    // so we need to another field SocketAccept in PER_IO_CONTEXT. When the outstanding
    // AcceptEx completes, this field is our connection socket handle.
    //

    //
    // data to be associated with every socket added to the IOCP
    //
    typedef struct _PER_SOCKET_CONTEXT {
        SOCKET                      Socket;

        LPFN_ACCEPTEX               fnAcceptEx;

        //
        //linked list for all outstanding i/o on the socket
        //
        PPER_IO_CONTEXT             pIOContext;  
        struct _PER_SOCKET_CONTEXT  *pCtxtBack; 
        struct _PER_SOCKET_CONTEXT  *pCtxtForward;
    } PER_SOCKET_CONTEXT, *PPER_SOCKET_CONTEXT;

    class NetworkManager{
    public:
        NetworkManager(const ServerInterface& server);
        ~NetworkManager();

        void start();
        void stop();

    private:
        const ServerInterface& server;
        WSAData wsaData;
        HANDLE iocp;

        const int maxThreads = 4;
        std::vector<std::thread> workers;
        DWORD threadId = 0;

        SOCKET listenSocket;
        SOCKET acceptSocket;
        PPER_SOCKET_CONTEXT lpPerSocketContext = NULL;

        void workerThread();
        void handleClientMessage();

        bool NetworkManager::CreateListenSocket();
    };
}

#endif
