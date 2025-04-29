/*
    MessageQueue.h

    MessageQueue interface file
*/

#ifndef MessageQueue_h
#define MessageQueue_h

#include <deque>
#include <vector>
#include <array>
#include <mutex>

namespace PrimeProcessor {

    class MessageQueue{
    private:

        // Mutex which locks both workQueue and inProgressQueue
        std::mutex queueMutex;

        // Queue of ranges to be searched
        std::deque<std::array<unsigned long long, 2>> workQueue;

        // Vector of ranges which are currently being searched
        std::vector<std::array<unsigned long long, 2>> rangesInProgress;

        // Vector of prime numbers found
        std::vector<unsigned long long> primesFound;

    public:

        // Dequeues a message returning a encoded byte array 
        std::array<unsigned long long, 2> dequeueWork();

        // Add new items to the workQueue
        void enqueueWork(std::vector<std::array<unsigned long long, 2>>& ranges);

        // Called by the SocketManager when a client is unable to finish a piece of work
        void searchFailed(std::array<unsigned long long, 2>& message);

        // Add primes found to a queue to be retreived and stored
        void enqueuePrimesFound(std::vector<unsigned long long>& primes);

        // Retrieve all primes found currently in vector
        std::vector<unsigned long long> retreivePrimesFound();

        // Returnes a vector of all ranges in progess and in the work queue
        std::vector<std::array<unsigned long long, 2>> emergencyDequeue();
    };
}

#endif
