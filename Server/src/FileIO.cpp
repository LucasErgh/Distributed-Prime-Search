#include "FileIO.h"
#include <array>

FileIO::FileIO() {
    rangesSearched = std::fstream(Primes_Searched_Path);
    primesFound = std::fstream(Primes_Found_Path, std::ios::app);
}

FileIO::~FileIO() {
    if (rangesSearched.is_open())
        rangesSearched.close();
    if (primesFound.is_open())
        primesFound.close();
}

void FileIO::readIn(std::vector<std::array<unsigned long long, 2>>& primesSearched){
    // Try to open files
    if(rangesSearched.fail()){
        rangesSearched.clear();
        rangesSearched.open(Primes_Searched_Path, std::ios::out);
        if(rangesSearched.fail())
            throw std::runtime_error("Failed to open \n");
    }
    if(primesFound.fail()){
        throw std::runtime_error("Failed to open\n");
    }

    // read ranges searched
    unsigned long long min, max;
    while(!rangesSearched.eof() && !rangesSearched.fail()){
        rangesSearched >> min >> max;
        if(!rangesSearched.fail())
            primesSearched.push_back( {min, max} );
    }
    if (primesSearched.empty()){
        primesSearched.push_back({1, 3});
        writePrimesFound({2, 3});
    }

    rangesSearched.clear();
    rangesSearched.close();
}

void FileIO::writePrimesFound(std::vector<unsigned long long> primes){
    for(const auto& cur : primes){
        primesFound << cur << '\n';
        if (!primesFound) {
            std::cerr << "Error writing to file!" << std::endl;
        }
    }
    primes.clear();
    primes.shrink_to_fit();
}

void FileIO::writeRangesSearched(std::vector<std::array<unsigned long long, 2>>& primesSearched){
    if (rangesSearched.is_open())
        rangesSearched.close();
    rangesSearched.open(Primes_Searched_Path, std::ios::out | std::ios::trunc);
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
