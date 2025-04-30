#include "ServerLogic.h"
#include "MySockets.h"

#include <iostream>
#include <string>
#include <thread>
#include <memory>

int main() {
    using namespace PrimeProcessor;

    std::shared_ptr<MessageQueue> messageQueue;

    ServerLogic server;
    std::thread serverThread(server.start(messageQueue));

    std::cin.get();

    server.stop();
    serverThread.join();

    return 0;
}
