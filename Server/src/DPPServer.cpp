#include "ServerLogic.h"
#include "NetworkManager.h"

#include <iostream>
#include <string>
#include <thread>
#include <memory>

int main() {
    using namespace PrimeProcessor;

    std::cout << '\n';

    MessageQueue* messageQueue = new MessageQueue();

    ServerLogic server(messageQueue);
    std::thread serverThread(&ServerLogic::start, &server);

    NetworkManager networkManager(messageQueue);
    networkManager.start();

    std::cin.get();

    networkManager.stop();

    server.stop();
    serverThread.join();

    delete messageQueue;

    return 0;
}
