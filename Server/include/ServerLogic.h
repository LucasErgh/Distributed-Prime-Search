#ifndef ServerLogic_h
#define ServerLogic_h

#include "MessageQueue.h"
#include "FileIO.h"
#include <fstream>
#include <stdexcept>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <cstring>

namespace PrimeProcessor{
    class ServerLogic {
    private:
        int searchSize = 100000; // (Max this can go is about 250,000 before header is too small)

        MessageQueue* messageQueue;

        std::atomic_bool stopFlag = false;

        FileIO io;

        std::vector<std::array<unsigned long long, 2>> primesSearched;

        unsigned long long largestSearched;

        // Stores set of primes on drive
        // Called when Prime set gets too big or the server is closing
        void storeToFile();

        // populate workQueue when it gets low
        void tryPopulateWorkQueue();

        // called after ranges searchd text file is read and primesSearched is populated
        // sorts the vector, merges sequential ranges, then adds missing ranges to workQueue
        void searchedNormalization(std::vector<std::array<unsigned long long, 2>>& primesSearched, std::vector<std::array<unsigned long long, 2>>& workQueue);

        void combineRangesBeforeWrite(std::vector<std::array<unsigned long long, 2>> &r);

    public:
        ServerLogic(MessageQueue* messageQueue);
        ~ServerLogic();

        void start();
        void stop();

        // called by the server manager to get the next range to search
        std::array<unsigned long long, 2> requestWork();

        // called by the server manager to return ranges found
        void primesReceived(std::vector<unsigned long long> primes, std::array<unsigned long long, 2>);
    
        void tryReceivingPrimes();
    };

}

#endif
