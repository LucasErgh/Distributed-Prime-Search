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
        std::vector<std::array<unsigned long long, 2>> inProgressQueue;

    public:

        // Dequeues a message returning a encoded byte array 
        std::vector<std::byte> dequeueWork();

        // Add a new item to the workQueue
        void enqueueWork(std::array<unsigned long long, 2> range);

        // Called by the SocketManager when a client is unable to finish a piece of work
        void enqueueFailedMessage(std::vector<std::byte> message);

        // Returnes a vector of all ranges in progess and in the work queue
        std::deque<std::array<unsigned long long, 2>> emergencyDequeue();
    };
}

#endif
