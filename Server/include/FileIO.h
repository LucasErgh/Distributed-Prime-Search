#ifndef FILEIO_H
#define FILEIO_H

#include <iostream>
#include <fstream>
#include <deque>
#include <vector>

class FileIO {
private:
    std::fstream rangesSearched;
    std::fstream primesFound;
public:
    FileIO();
    ~FileIO();

    void readIn(std::vector<std::array<unsigned long long, 2>>& primesSearched);
    void writePrimesFound(std::vector<unsigned long long> primes);
    void writeRangesSearched(std::vector<std::array<unsigned long long, 2>>& primesSearched);
};

#endif
