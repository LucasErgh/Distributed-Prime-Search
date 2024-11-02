
#include "ServerLogic.h"

#include <algorithm>

namespace PrimeProcessor {
 
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
        int min, max;
        while(!rangesSearched.eof() && !rangesSearched.fail()){
            rangesSearched >> min >> max;
            if(rangesSearched.fail()){
                primesSearched.push_back(std::make_pair(1,1));
            } else {
                primesSearched.push_back(std::make_pair(min, max));
            }
        }
        rangesSearched.clear();

        // sorts the vector, merges sequential ranges, then adds missing ranges to workQueue
        searchedNormalization();

        largestSearched = primesSearched.back().second;

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
            rangesSearched << cur.first << " " << cur.second;
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
        // ull max = *primes.end();
        // int searchSize = (max <= 100) ? 1000 : (max^2 + 10^5*(max))/max^2 + 1; 
        int searchSize = 100;

        for (int i = workQueue.size(); i < newSize; i++){
            workQueue.push_back(std::make_pair(largestSearched+1, largestSearched + searchSize));
        }

        largestSearched = largestSearched + searchSize;
        workQueueMutex.unlock();
    }

    // finds gaps in rangesSearched at the start of the program
    void ServerLogic::searchedNormalization(){
        primesSearchedMutex.lock();
        workQueueMutex.lock();

        std::sort(primesSearched.begin(), primesSearched.end(), [](auto &left, auto &right){return left.second < right.second;});
        
        for(auto i = primesSearched.begin(); i != primesSearched.end() && (i + 1) != primesSearched.end();  i++){
            if ((i+1)->first - i->second <= 1){
                range pairUnion(i->first, (i+1)->second);
                primesSearched.erase(i, (i+1));
                primesSearched.insert((i-1), pairUnion);
                --i;
            }
            else {
                range missing((i->second) + 1, (i + 1)->first-1);
                workQueue.push_front(missing);
            }
        }

        primesSearchedMutex.unlock();
        workQueueMutex.unlock();
    }

    void ServerLogic::foundPrimes(std::set<ull> p){
        primesMutex.lock();
        primes.insert(p.begin(), p.end());
        primesMutex.unlock();
    }

}