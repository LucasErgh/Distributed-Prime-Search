#ifndef ServerLogic_h
#define ServerLogic_h

#include "MessageQueue.h"
#include <fstream>
#include <stdexcept>
#include <memory>
#include <vector>
#include <deque>
#include <set>
#include <mutex>
#include <thread>

namespace PrimeProcessor{
    class ServerLogic {
    private:
        const std::shared_ptr<MessageQueue> messageQueue;

        std::atomic_bool stopFlag = false;

        const std::string rangeFile = "../Ranges_Searched.txt";
        const std::string primeFile = "../Primes_Found.txt";
        std::fstream rangesSearched;
        std::fstream primesFound;

        std::vector<std::array<unsigned long long, 2>> primesSearched;

        unsigned long long largestSearched;

        // Stores set of primes on drive
        // Called when Prime set gets too big or the server is closing
        void storeToFile(std::vector<std::array<unsigned long long, 2>> primesSearched, std::vector<unsigned long long> primes);

        // populate workQueue when it gets low
        void tryPopulateWorkQueue();

        // called after ranges searchd text file is read and primesSearched is populated
        // sorts the vector, merges sequential ranges, then adds missing ranges to workQueue
        void searchedNormalization(std::vector<std::array<unsigned long long, 2>>& primesSearched, std::vector<std::array<unsigned long long, 2>>& workQueue);

        void combineRangesBeforeWrite(std::vector<std::array<unsigned long long, 2>> &r);

    public:
        ServerLogic(std::shared_ptr<MessageQueue> messageQueue);
        ~ServerLogic();

        bool start();
        void stop();

        // called by the server manager to get the next range to search
        std::array<unsigned long long, 2> requestWork();

        // called by the server manager to return ranges found
        void primesReceived(std::vector<unsigned long long> primes, std::array<unsigned long long, 2>);
    
        void tryReceivingPrimes();
    };

}

#endif
