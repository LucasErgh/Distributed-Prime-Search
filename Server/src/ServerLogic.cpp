
#include "ServerLogic.h"
#include "MySockets.h"
#include "FileIO.h"

#include <algorithm>

#include <iostream>

namespace PrimeProcessor {
 
    void ServerLogic::returnRange(std::array<unsigned long long, 2> arr){
        WIPQueueMutex.lock();
        workQueueMutex.lock();

        auto r = std::find(WIPQueue.begin(), WIPQueue.end(), arr);
        if(r != WIPQueue.end()){
            workQueue.push_back(std::array<unsigned long long, 2>(*r));
            WIPQueue.erase(r);
            WIPQueue.shrink_to_fit();
        }
        // To-Do if it doesn't exist in the WIPQueue or the workQueue, remove it from primesSearched if it is within one of those ranges

        WIPQueueMutex.unlock();
        workQueueMutex.unlock();
    }

    ServerLogic::ServerLogic(): manager(new SocketManager(this)), rangesSearched(std::fstream(rangeFile)), primesFound(std::fstream(primeFile, std::ios::app)){
        // read data from file
        readIn(rangesSearched, primesFound, primesSearched);

        // sorts the vector, merges sequential ranges, then adds missing ranges to workQueue
        searchedNormalization();
        largestSearched = primesSearched.back()[1];
        populateWorkQueue();
    }

    ServerLogic::~ServerLogic(){}

    bool ServerLogic::start(){
        manager->start();
        return true;
    }

    void ServerLogic::stop(){
        manager->stop();
        storeToFile();
        primesFound.close();
        if(!primesMutex.try_lock() || !primesSearchedMutex.try_lock()){
            throw "no no no";
        }
    }

    void ServerLogic::storeToFile(){

        combineRangesBeforeWrite(primesSearched);
        primesSearchedMutex.lock();
        writeRangesSearched(rangesSearched, primesSearched);
        primesSearchedMutex.unlock();

        primesMutex.lock();
        writePrimesFound(primesFound, primes);
        primesMutex.unlock();
    }

    void ServerLogic::populateWorkQueue(){
        workQueueMutex.lock();
        int newSize = (workQueue.size() < 5) ? 10 : workQueue.size() * 2;
        
        //// number of digits per search set
        // unsigned long long max = *primes.end();
        // int searchSize = (max <= 100) ? 1000 : (max^2 + 10^5*(max))/max^2 + 1; 
        int searchSize = 100;

        for (int i = workQueue.size(); i < newSize; i++){
            workQueue.push_front( {largestSearched + 1, largestSearched + searchSize} );
            largestSearched = largestSearched + searchSize;
        }
        workQueueMutex.unlock();
    }

    // finds gaps in rangesSearched at the start of the program
    void ServerLogic::combineRangesBeforeWrite(std::deque<std::array<unsigned long long, 2>> &r){
        std::deque<std::array<unsigned long long, 2>> mergedRanges;        

        std::sort(r.begin(), r.end(), [](auto &left, auto &right){return left[1] < right[1];});
        
        
        for(const auto &cur : r){
            if(mergedRanges.empty()){
                mergedRanges.push_back(cur);
            } else if(cur.at(0) <= mergedRanges.back().at(1) + 1){
                mergedRanges.back()[1] = std::max(mergedRanges.back()[1], cur.at(1));
            }
        }

       r.swap(mergedRanges);
    }

    // finds gaps in rangesSearched at the start of the program
    void ServerLogic::searchedNormalization(){
        primesSearchedMutex.lock();
        workQueueMutex.lock();

        std::sort(primesSearched.begin(), primesSearched.end(), [](auto &left, auto &right){return left[0] < right[0];});
        
        for(auto i = primesSearched.begin(); i != primesSearched.end() && (i + 1) != primesSearched.end();  i++){

            if (((i+1)->at(0) - i->at(1)) <= 1){
                std::array<unsigned long long, 2> pairUnion = {i->at(0), (i+1)->at(1)};
                i = primesSearched.erase(i, (i+2));
                i = primesSearched.insert(i, pairUnion);
                i--;
            }
            else {
                std::array<unsigned long long, 2> missing = {(i->at(1)) + 1, (i + 1)->at(0) - 1};
                workQueue.push_front(missing);
            }
        }

        primesSearchedMutex.unlock();
        workQueueMutex.unlock();
    }

    void ServerLogic::foundPrimes(std::vector<unsigned long long> p, std::array<unsigned long long, 2> r){
        // To-Do remove range from WIPQueue
        WIPQueueMutex.lock();
        auto i = std::find_if(WIPQueue.begin(), WIPQueue.end(), [&r](auto &pair){ return pair[0] == r[0] && pair[1] == r[1]; });
        if(i != WIPQueue.end()){
            primesSearchedMutex.lock();
            primesSearched.push_front({(*i)[0], (*i)[1]});
            WIPQueue.erase(i);
        }
        primesSearchedMutex.unlock();
        WIPQueueMutex.unlock();
        
        primesMutex.lock();
        primes.insert(primes.end(), p.begin(), p.end());
        primesMutex.unlock();
    }

    std::array<unsigned long long, 2> ServerLogic::getRange(){
        workQueueMutex.lock();
        WIPQueueMutex.lock();

        std::array<unsigned long long, 2> r(workQueue.back());
        workQueue.pop_back();
        workQueue.shrink_to_fit();
        WIPQueue.push_front(r);

        workQueueMutex.unlock();
        WIPQueueMutex.unlock();

        if(workQueue.size() < 10) populateWorkQueue();

        return std::array<unsigned long long, 2>{r[0], r[1]};
    }

}
