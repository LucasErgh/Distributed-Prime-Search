#include "FileIO.h"
#include <array>

void readIn(std::fstream& rangesSearched, std::fstream& primesFound, std::vector<std::array<unsigned long long, 2>>& primesSearched){
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
    }
    if (primesSearched.empty())
        primesSearched.push_back({1, 1});

    rangesSearched.clear();
    rangesSearched.close();
}

void writePrimesFound(std::fstream& primesFound, std::vector<unsigned long long> primes){
    for(const auto& cur : primes){
        primesFound << cur << " " << '\n';
        if (!primesFound) {
            std::cerr << "Error writing to file!" << std::endl;
        }
    }
    primes.clear();
    primes.shrink_to_fit();
}

void writeRangesSearched(std::fstream& rangesSearched, std::vector<std::array<unsigned long long, 2>>& primesSearched){
    rangesSearched.open(rangeFile, std::ios::out | std::ios::trunc);
    if(rangesSearched.fail()){
        throw "Fail";
    }
    for(auto cur : primesSearched){
        rangesSearched << cur[0] << " " << cur[1] << '\n';
    }
    rangesSearched.close();
    primesSearched.clear();
    primesSearched.shrink_to_fit();
}
