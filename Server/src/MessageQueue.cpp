/*
    MessageQueue.cpp

    Implementation file for MessageQueue
*/

#include "MessageQueue.h"

namespace PrimeProcessor {
    // Dequeues a message returning a encoded byte array 
    std::vector<std::byte> MessageQueue::dequeueWork(){

    }

    // Add a new item to the workQueue
    void MessageQueue::enqueueWork(std::array<unsigned long long, 2> range){

    }

    // Called by the SocketManager when a client is unable to finish a piece of work
    void MessageQueue::messageFailed(std::vector<std::byte> message){

    }

    // Add primes found to a queue to be retreived and stored
    void MessageQueue::enqueuePrimesFound(std::vector<unsigned long long>){

    }

    // Returnes a vector of all ranges in progess and in the work queue
    std::deque<std::array<unsigned long long, 2>> MessageQueue::emergencyDequeue(){

    }
}