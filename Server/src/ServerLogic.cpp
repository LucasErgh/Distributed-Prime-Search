
#include "ServerLogic.h"
#include "MySockets.h"

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
        }
        // To-Do if it doesn't exist in the WIPQueue or the workQueue, remove it from primesSearched if it is within one of those ranges

        WIPQueueMutex.unlock();
        workQueueMutex.unlock();
    }

    ServerLogic::ServerLogic(): manager(new SocketManager(this)), rangesSearched(std::fstream(rangeFile)), primesFound(std::fstream(primeFile, std::ios::app)){
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
            if(!rangesSearched.fail())
                primesSearched.push_back( {min, max} );
            else primesSearched.push_back( {1, 1});
        }
        rangesSearched.clear();

        // sorts the vector, merges sequential ranges, then adds missing ranges to workQueue
        searchedNormalization();

        if (!primesSearched.empty())
            largestSearched = primesSearched.back()[1];
        else largestSearched = 0;

        populateWorkQueue();
    }

    ServerLogic::~ServerLogic(){}

    bool ServerLogic::start(){
        manager->start();
        return true;
    }

    void ServerLogic::stop(){
        manager->stop();
        primesSearchedMutex.lock();
        combineRangesBeforeWrite(primesSearched);
        if (!primesSearched.empty())
            for(const auto& cur : primesSearched)
                rangesSearched << cur[0] << " " << cur[1] << '\n';
        rangesSearched.close();
        primesSearchedMutex.unlock();
        storePrimesInFile();
    }

    void ServerLogic::storePrimesInFile(){
        primesMutex.lock();
        for(const auto& cur : primes){
            primesFound << cur << " " << '\n';
            if (!primesFound) {
                std::cerr << "Error writing to file!" << std::endl;
            }
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
            workQueue.push_front( {largestSearched + 1, largestSearched + searchSize} );
            largestSearched = largestSearched + searchSize;
        }
        workQueueMutex.unlock();
    }

    // finds gaps in rangesSearched at the start of the program
    void ServerLogic::combineRangesBeforeWrite(std::vector<std::array<unsigned long long, 2>> &r){
        std::sort(r.begin(), r.end(), [](auto &left, auto &right){return left[1] < right[1];});
        
        for(auto i = r.begin(); i != r.end() && (i + 1) != r.end();  i++){
            if ((i+1)->at(0) - i->at(1) <= 1){
                std::array<unsigned long long, 2> pairUnion = {i->at(0), (i+1)->at(1)};
                r.erase(i, (i+1));
                r.insert((i-1), pairUnion);
                --i;
            }
        }

        std::sort(r.begin(), r.end(), [](auto &left, auto &right){return left[1] < right[1];});
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
            primesSearched.push_back({(*i)[0], (*i)[1]});
            primesSearchedMutex.unlock();
            WIPQueue.erase(i);
        }
        WIPQueueMutex.unlock();
        
        primesMutex.lock();
        primes.insert(primes.end(), p.begin(), p.end());
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