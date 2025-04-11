#ifndef SERVERINTERFACE_H
#define SERVERINTERFACE_H

#include <array>
#include <vector>

namespace PrimeProcessor{
    class ServerInterface{
        public:

            virtual std::array<unsigned long long, 2> requestWork() = 0;

            virtual void workFailed(std::array<unsigned long long, 2>) = 0;

            virtual void primesReceived(std::vector<unsigned long long> primes, std::array<unsigned long long, 2>) = 0;
    };
}

#endif
