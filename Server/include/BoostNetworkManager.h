#ifndef BOOSTNETWORKMANAGER_H
#define BOOSTNETWORKMANAGER_H

#include "ServerInterface.h"
#include "Serialization.h"
#include <asio.hpp>

using asio::ip::tcp;

namespace PrimeProcessor{
    class BoostManager{
    public:
        BoostManager(ServerInterface& server);
        ~BoostManager();

        void start();
        void stop();
    private:
        ServerInterface& server;
        asio::io_context io_context;
        tcp::acceptor acceptor;
        tcp::socket socket;

        void AcceptHandler(const asio::error_code& error);
        void clientConnection();
        void getMessageHeader();
        void getMessageBody(int msgType, size_t payloadSize);
        void sendMessage();
    };
}

#endif
