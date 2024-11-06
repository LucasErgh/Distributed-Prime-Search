
#include "ServerLogic.h"
#include "MySockets.h"

#include <algorithm>

namespace PrimeProcessor {
 
    void ServerLogic::returnRange(std::array<unsigned long long, 2> arr){
        WIPQueueMutex.lock();
        workQueueMutex.lock();

        auto r = std::find(WIPQueue.begin(), WIPQueue.end(), arr);
        if(r != WIPQueue.end()){
            workQueue.push_back(std::array<unsigned long long, 2>(*r));
            WIPQueue.erase(r);
        }
        // To-Do if it doesn't exist in the WIPQueue or the workQueue, remove it from primesSearched if it is within one of those ranges

        WIPQueueMutex.unlock();
        workQueueMutex.unlock();
    }

    ServerLogic::ServerLogic(): manager(new SocketManager(this)), rangesSearched(std::fstream(rangeFile)), primesFound(std::fstream(primeFile, std::ios::out)){
        // Try to open files
        if(rangesSearched.fail()){
            rangesSearched.clear();
            rangesSearched.open(rangeFile, std::ios::out);
            if(rangesSearched.fail())
                throw std::runtime_error("Failed to open " + rangeFile + '\n');
        }
        if(primesFound.fail()){
            throw std::runtime_error("Failed to open " + primeFile + '\n');
        }

        // read ranges searched
        unsigned long long min, max;
        while(!rangesSearched.eof() && !rangesSearched.fail()){
            rangesSearched >> min >> max;
            if(rangesSearched.fail()){
                primesSearched.push_back( {1, 1} );
            } else {
                primesSearched.push_back( {min, max} );
            }
        }
        rangesSearched.clear();

        // sorts the vector, merges sequential ranges, then adds missing ranges to workQueue
        searchedNormalization();

        largestSearched = primesSearched.back()[1];

        populateWorkQueue();
    }

    ServerLogic::~ServerLogic(){
        stop();
    }

    bool ServerLogic::start(){
        manager->start();
        return true;
    }

    void ServerLogic::stop(){
        manager->stop();
        for(const auto& cur : primesSearched){
            rangesSearched << cur[0] << " " << cur[1];
        }
        storePrimes();
    }

    void ServerLogic::storePrimes(){
        primesMutex.lock();
        for(const auto& cur : primes){
            primesFound << cur << " " << '\n';
        }
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
            workQueue.push_back( {largestSearched + 1, largestSearched + searchSize} );
            largestSearched = largestSearched + searchSize;
        }
        workQueueMutex.unlock();
    }

    // finds gaps in rangesSearched at the start of the program
    void ServerLogic::searchedNormalization(){
        primesSearchedMutex.lock();
        workQueueMutex.lock();

        std::sort(primesSearched.begin(), primesSearched.end(), [](auto &left, auto &right){return left[1] < right[1];});
        
        for(auto i = primesSearched.begin(); i != primesSearched.end() && (i + 1) != primesSearched.end();  i++){
            if ((i+1)->at(0) - i->at(1) <= 1){
                std::array<unsigned long long, 2> pairUnion = {i->at(0), (i+1)->at(1)};
                primesSearched.erase(i, (i+1));
                primesSearched.insert((i-1), pairUnion);
                --i;
            }
            else {
                std::array<unsigned long long, 2> missing = {(i->at(1)) + 1, (i + 1)->at(0) - 1};
                workQueue.push_front(missing);
            }
        }

        primesSearchedMutex.unlock();
        workQueueMutex.unlock();
    }

    void ServerLogic::foundPrimes(std::vector<unsigned long long> p){
        // To-Do remove range from WIPQueue
        primesMutex.lock();
        primes.insert(p.begin(), p.end());
        primesMutex.unlock();
    }

    std::array<unsigned long long, 2> ServerLogic::getRange(){
        workQueueMutex.lock();
        WIPQueueMutex.lock();

        std::array<unsigned long long, 2> r = workQueue.back();
        workQueue.pop_back();
        WIPQueue.push_back(r);

        workQueueMutex.unlock();
        WIPQueueMutex.unlock();

        if(workQueue.size() < 10) populateWorkQueue();

        return r;
    }

}