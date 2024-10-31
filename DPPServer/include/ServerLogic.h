#pragma once

#include "MySockets.h"

#include <stdexcept>

#include<deque>
#include<set>
#include<fstream>

namespace PrimeProcessor{

typedef std::pair<int, int> range;

    class ServerLogic{
    private:
        const std::string rangeFile = "Range_Searched";
        const std::string primeFile = "Primes Found";
        std::fstream rangeSearched;
        std::fstream primesFound;

        PrimeProcessor::SocketManager socks;

        std::deque<range> workQueue;
        std::mutex workQueueMutex;

        range primesSearched;
        std::mutex primesSearchedMutex;

        std::set<int> primes;
        std::mutex primesMutex;

        // Stores set of primes on drive
        // Called when Prime set gets too big or the server is closing
        void storePrimes(std::set<int> p);
        
        // populate workQueue when it gets low
        void populateWorkQueue();

    public:
        ServerLogic();
        ~ServerLogic();

        bool start();
        void stop();

        void foundPrimes();
    };

}