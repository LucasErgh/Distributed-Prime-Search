#ifndef ServerLogic_h
#define ServerLogic_h

#include "ServerInterface.h"

#include <fstream>
#include <stdexcept>

#include <memory>
#include <vector>
#include <deque>
#include <set>

#include <mutex>
#include <thread>

namespace PrimeProcessor{
    class ServerLogic : public ServerInterface{
    private:

        const std::string rangeFile = "../Ranges_Searched.txt";
        const std::string primeFile = "../Primes_Found.txt";
        std::fstream rangesSearched;
        std::fstream primesFound;

        // To-Do Try to consider using atomic variables rather than mutexs

        std::deque<std::array<unsigned long long, 2>> workQueue;
        std::mutex workQueueMutex;

        std::deque<std::array<unsigned long long, 2>> WIPQueue;
        std::mutex WIPQueueMutex;

        std::deque<std::array<unsigned long long, 2>> primesSearched;
        std::mutex primesSearchedMutex;
        unsigned long long largestSearched;

        std::vector<unsigned long long> primes;
        std::mutex primesMutex;

        // Stores set of primes on drive
        // Called when Prime set gets too big or the server is closing
        void storeToFile();

        // populate workQueue when it gets low
        void populateWorkQueue();

        // called after ranges searchd text file is read and primesSearched is populated
        // sorts the vector, merges sequential ranges, then adds missing ranges to workQueue
        void searchedNormalization();

        void combineRangesBeforeWrite(std::deque<std::array<unsigned long long, 2>> &r);

    public:
        ServerLogic();
        ~ServerLogic();

        bool start();
        void stop();

        // called by the server manager to get the next range to search
        std::array<unsigned long long, 2> requestWork() override;

        // called when client error occurs, range gets moved from WIPQueue to WorkQueue
        void workFailed(std::array<unsigned long long, 2>) override;

        // called by the server manager to return ranges found
        void primesReceived(std::vector<unsigned long long> primes, std::array<unsigned long long, 2>) override;
    };

}

#endif
