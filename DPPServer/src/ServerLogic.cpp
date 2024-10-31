
#include "ServerLogic.h"

namespace PrimeProcessor {
 
    ServerLogic::ServerLogic(){
        // Try to open files
        rangeSearched.open(rangeFile);
        if(rangeSearched.fail())
            throw std::runtime_error("Failed to open " + rangeFile + '\n');

        primesFound.open(primeFile);
        if(primesFound.fail())
            throw std::runtime_error("Failed to open " + primeFile + '\n');

        

    }

    ServerLogic::~ServerLogic(){

    }

    bool ServerLogic::start(){
        return true; // temporary so I can build before implimentation
    }

    void ServerLogic::stop(){

    }

    void ServerLogic::storePrimes(std::set<int> p){

    }
    void ServerLogic::populateWorkQueue(){

    }

}