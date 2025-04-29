/*
    MessageQueue.cpp

    Implementation file for MessageQueue
*/

#include "MessageQueue.h"
#include <algorithm>

namespace PrimeProcessor {

    // Dequeues a message returning a encoded byte array 
    std::array<unsigned long long, 2> MessageQueue::dequeueWork(){
        std::lock_guard<std::mutex> lock(queueMutex);

        std::array<unsigned long long, 2> back = workQueue.back();
        rangesInProgress.push_back(back);
        workQueue.pop_back();
        return back;
    }

    // Add new items to the workQueue
    void MessageQueue::enqueueWork(std::vector<std::array<unsigned long long, 2>>& ranges){
        std::lock_guard<std::mutex> lock(queueMutex);

        for (auto& cur : ranges) {
            workQueue.push_front(std::move(cur));
        }
    }

    // Called by the SocketManager when a client is unable to finish a piece of work
    void MessageQueue::searchFailed(std::array<unsigned long long, 2>& message){
        std::lock_guard<std::mutex> lock(queueMutex);

        rangesInProgress.erase(std::find(rangesInProgress.begin(), rangesInProgress.end(), message));
        workQueue.push_back(std::move(message));
    }

    // Add primes found to a queue to be retreived and stored
    void MessageQueue::enqueuePrimesFound(std::vector<unsigned long long>& primes){
        std::lock_guard<std::mutex> lock(queueMutex);

        primesFound.insert(primesFound.end(), primes.begin(), primes.end());
    }

    // Retrieve all primes found currently in vector
    std::vector<unsigned long long> MessageQueue::retreivePrimesFound(){
        std::lock_guard<std::mutex> lock(queueMutex);

        std::vector<unsigned long long> primesToReturn = std::move(primesFound);
        return primesToReturn;
    }

    // Returnes a vector of all ranges in progess and in the work queue
    std::vector<std::array<unsigned long long, 2>> MessageQueue::emergencyDequeue(){
        std::lock_guard<std::mutex> lock(queueMutex);

        std::vector<std::array<unsigned long long, 2>> allWorkRanges;

        allWorkRanges.reserve(rangesInProgress.size() + workQueue.size());

        allWorkRanges.insert(allWorkRanges.end(), 
                             std::make_move_iterator(workQueue.begin()), 
                             std::make_move_iterator(workQueue.end()));

        allWorkRanges.insert(allWorkRanges.end(),
                             std::make_move_iterator(rangesInProgress.begin()),
                             std::make_move_iterator(rangesInProgress.end()));

        return allWorkRanges;
    }

}