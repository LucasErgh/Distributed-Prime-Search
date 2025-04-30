
#include "ServerLogic.h"
#include "FileIO.h"

#include <algorithm>
#include <array>

#include <iostream>

namespace PrimeProcessor {

    ServerLogic::ServerLogic(MessageQueue* messageQueue) :
        rangesSearched(std::fstream(rangeFile)),
        primesFound(std::fstream(primeFile, std::ios::app)),
        messageQueue(messageQueue){
            std::cerr << "Test inside ServerLogic Constructor\n";
    }

    ServerLogic::~ServerLogic(){}

    void ServerLogic::start(){
        // read data from file
        readIn(rangesSearched, primesFound, primesSearched);

        // sorts the vector, merges sequential ranges, then adds missing ranges to workQueue
        std::vector<std::array<unsigned long long, 2>> workQueue;
        workQueue.clear();

        searchedNormalization(primesSearched, workQueue);

        largestSearched = primesSearched.back()[1];

        if (workQueue.size() != 0)
            messageQueue->enqueueWork(workQueue);

        while(!stopFlag) {
            tryPopulateWorkQueue();
            tryReceivingPrimes();
        }

        // Now shutdown server
        storeToFile(messageQueue->pretreivePrimesSearched(), messageQueue->retreivePrimesFound());
        primesFound.close();

    }

    void ServerLogic::stop(){
        stopFlag = true;
    }

    void ServerLogic::storeToFile(std::vector<std::array<unsigned long long, 2>> primesSearched, std::vector<unsigned long long> primes){

        combineRangesBeforeWrite(primesSearched);
        writeRangesSearched(rangesSearched, primesSearched);

        writePrimesFound(primesFound, primes);
    }

    void ServerLogic::tryPopulateWorkQueue(){
        std::vector<std::array<unsigned long long, 2>> workQueue;
        if (messageQueue->needsWorkEnqueued()){

            //// number of digits per search set
            // unsigned long long max = *primes.end();
            // int searchSize = (max <= 100) ? 1000 : (max^2 + 10^5*(max))/max^2 + 1; 
            int searchSize = 100;

            for (int i = workQueue.size(); i < 100; i++){
                workQueue.push_back( {largestSearched + 1, largestSearched + searchSize} );
                largestSearched = largestSearched + searchSize;
            }

            messageQueue->enqueueWork(workQueue);
        }
    }

    // finds gaps in rangesSearched at the start of the program
    void ServerLogic::combineRangesBeforeWrite(std::vector<std::array<unsigned long long, 2>> &r){
        std::vector<std::array<unsigned long long, 2>> mergedRanges;        

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
    void ServerLogic::searchedNormalization(std::vector<std::array<unsigned long long, 2>>& primesSearched, std::vector<std::array<unsigned long long, 2>>& workQueue){
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
                workQueue.push_back(missing);
            }
        }
    }

    void ServerLogic::tryReceivingPrimes(){
        if (messageQueue->primesToRetreive()){
            writePrimesFound(primesFound, messageQueue->retreivePrimesFound());
        }
    }

}
