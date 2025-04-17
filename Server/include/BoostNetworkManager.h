#ifndef BOOSTNETWORKMANAGER_H
#define BOOSTNETWORKMANAGER_H

#include "ServerInterface.h"
#include "Serialization.h"
#include <asio.hpp>

namespace PrimeProcessor{
    class BoostManager{
    public:
        BoostManager(ServerInterface& server);
        ~BoostManager();

        void start();
        void stop();
    private:
        
    };
}

#endif
