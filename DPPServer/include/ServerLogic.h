#pragma once

#include "MySockets.h"

#include<deque>

typedef std::pair<int, int> range;

class ServerLogic{
private:
    MySockets::SocketManager socks;

    std::deque<range> workQueue;

public:
    ServerLogic();
    ~ServerLogic();

    bool start();
    void stop();

    void storePrimes(std::vector<int> p);
    void populateWorkQueue();
};