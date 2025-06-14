#include "ServerLogic.h"
#include "NetworkManager.h"

#include <iostream>
#include <string>
#include <thread>
#include <memory>

int main() {
    using namespace PrimeProcessor;

    MessageQueue* messageQueue = new MessageQueue();

    ServerLogic server(messageQueue);
    std::thread serverThread(&ServerLogic::start, &server);

    NetworkManager networkManager(messageQueue);
    try{
        networkManager.start();
    } catch (std::string& e) {
        std::cerr << e << '\n';
    } catch (...) {
        std::cerr << "Caught in main()";
    }

    std::cin.get();

    networkManager.stop();

    server.stop();
    serverThread.join();

    delete messageQueue;

    return 0;
}
