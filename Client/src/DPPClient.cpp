// DPPClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "config.h"
#include "ConnectionHandler.h"

#include <iostream>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

int main()
{
    Connection c;
    c.start();

    std::cin.get();

    c.stop();

    return 0;
}
