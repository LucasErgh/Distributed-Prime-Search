#include "ServerLogic.h"
#include "MySockets.h"

#include <iostream>
#include <string>
#include <thread>
#include <memory>

int main() {
    using namespace PrimeProcessor;

    MessageQueue* messageQueue = new MessageQueue();

    ServerLogic server(messageQueue);
    std::thread serverThread(&ServerLogic::start, &server);

    SocketManager socketManager(messageQueue);
    socketManager.start();

    std::cin.get();

    socketManager.stop();

    server.stop();
    serverThread.join();

    delete messageQueue;

    return 0;
}
