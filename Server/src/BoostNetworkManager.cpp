#include "BoostNetworkManager.h"
#include "Serialization.h"
#include <vector>
#include <iostream>

using asio::ip::tcp;

namespace PrimeProcessor {

    BoostManager::BoostManager(ServerInterface& server) :
        server(server),
        io_context(),
        acceptor(io_context, tcp::endpoint(tcp::v4(), std::stoi(DEFAULT_PORT))),
        socket(io_context)
    {
    }

    BoostManager::~BoostManager() {
        // Empty destructor implementation
    }

    void BoostManager::start() {
        acceptor.async_accept(io_context, std::bind(BoostManager::AcceptHandler, this, asio::placeholders::error));
    }

    void BoostManager::stop() {
        // Empty stop method implementation
    }

    void BoostManager::AcceptHandler(const asio::error_code& error){
        if (!error){
            getMessageHeader();
        }
        start();
    }

    void BoostManager::getMessageHeader(){
        uint8_t header[3];
        int msgType;
        uint16_t payloadSize;
        int bytesReceived;

        asio::async_read(socket, asio::buffer(header, 3), [&](const asio::error_code& error, std::size_t bytes_transferred){
            if (!error){
                bytesReceived += bytes_transferred;

                msgType = readMsg(header, payloadSize);
                assert(msgType);

                getMessageBody(msgType, payloadSize);
            }
        });

    }

    void BoostManager::getMessageBody(int msgType, size_t payloadSize){
        size_t payloadBytes = payloadSize * sizeof(unsigned long long);

        std::vector<std::byte> payload(payloadBytes);
        if (payloadBytes == 0){
            // handle
        }

        asio::async_read(socket, asio::buffer(payload, payloadBytes), [&](const asio::error_code& error, std::size_t bytes_transferred){
            // To-Do handdle errors
            std::vector<unsigned long long> primes(payloadSize);
            std::fill(primes.begin(), primes.end(), 0);
            if (!readMsg(payload, payloadSize, primes)){
                // handle errors
            }

            sendMessage();
        });
    }

    void BoostManager::sendMessage(){
        // To-Do handle errors
        auto range = server.requestWork();
        auto msg = createMsg(range);
        asio::async_write(
            socket,
            asio::buffer(msg.data(), msg.size()), 
            [this](const asio::error_code& error, std::size_t bytes_transfered){
                if (error){

                } else {

                }
            }
        );
    }

}