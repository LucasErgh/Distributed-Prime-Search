
#include "ServerLogic.h"

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

    void ServerLogic::storePrimes(std::set<int> p){

    }
    void ServerLogic::populateWorkQueue(){

    }

}