
#include "ServerLogic.h"

#include <algorithm>

namespace PrimeProcessor {
 
    ServerLogic::ServerLogic(): manager(new SocketManager()), rangesSearched(std::fstream(rangeFile)), primesFound(std::fstream(primeFile, std::ios::out)){
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
    }

    ServerLogic::~ServerLogic(){

    }

    bool ServerLogic::start(){
        manager->start();
        return true;
    }

    void ServerLogic::stop(){

    }

    void ServerLogic::storePrimes(std::set<ull> p){

    }
    void ServerLogic::populateWorkQueue(){

        int newSize = (workQueue.size() < 5) ? 10 : workQueue.size() * 2;
        
        // try to join pairs in primesSearched vector
        // if there is a missing range in primesSearched, add it to workQueue

        // clever way to sort vector of pairs I saw on cppreference std::sort page using lambda expressions
        std::sort(primesSearched.begin(), primesSearched.end(), [](auto &left, auto &right){return left.second < right.second});
        
        for(auto i = primesSearched.begin(); i != primesSearched.end() && (i + 1) != primesSearched.end();  i++){
            if ((i+1)->first - i->second <= 1){
                range pairUnion(i->first, (i+1)->second);
                primesSearched.erase(i, (i+1));
                primesSearched.insert((i-1), pairUnion);
                --i;
            }
            else {
                range missing((i->second) + 1, (i + 1)->first-1);
                // make sure its not in WIPQueue or workQueue
                if(std::find(WIPQueue.begin(), WIPQueue.end(), missing) == WIPQueue.end() &&
                    std::find(workQueue.begin(), workQueue.end(), missing) == workQueue.end()) 
                    workQueue.push_front(missing);
            }
        }

        if(workQueue.size() >= newSize) return;

        // now sort workQueue using same method (but in decending order)
        std::sort(workQueue.begin(), workQueue.end(), [](auto &left, auto &right){return left.second > right.second});

        // check for missing range between primesSearched and workQueue (and make sure its not in WIPQueue)
        if(workQueue.front().first - primesSearched.back().second)
        auto i = primesSearched.back().second + 1;

        // number of digits per search set
        // ull max = *primes.end();
        // int i = (max <= 100) ? 1000 : (max^2 + 10^5*(max))/max^2 + 1; 
    
        int searchSize = 100;

        // Find next range and assume there is no gap between primesSearched and primesBeingSearched
        if (workQueue.empty()){
            
        }
        if (WIPQueue.empty()) {
            
        }
    }

}