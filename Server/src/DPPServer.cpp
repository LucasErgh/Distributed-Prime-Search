#include "ServerLogic.h"
#include "MySockets.h"

#include <iostream>
#include <string>
#include <thread>
#include <memory>

int main() {
    using namespace PrimeProcessor;

    std::shared_ptr<MessageQueue> messageQueue = std::make_shared<MessageQueue>();
    if (!messageQueue.get())
        std::cerr << "We found the problem\n";

    std::cerr << "\nTest in main() after messageQueue declaration\n";

    ServerLogic server(messageQueue);
    std::thread serverThread(&ServerLogic::start, &server);

    std::cerr << "Test after serverThread started\n";

    SocketManager socketManager(messageQueue);
    std::cerr << "test after socketManager is initialized\n";
    socketManager.start();

    std::cin.get();

    socketManager.stop();

    server.stop();
    serverThread.join();

    return 0;
}
