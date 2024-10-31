#pragma once

#include "MySockets.h"


class ServerLogic{
private:
    MySockets::SocketManager socks;    

public:
    ServerLogic();

    bool start();
    void stop();

};