#ifndef FILEIO_H
#define FILEIO_H

#include <iostream>
#include <fstream>
#include <deque>
#include <vector>

const std::string rangeFile = "../Ranges_Searched.txt";
const std::string primeFile = "../Primes_Found.txt";

void readIn(std::fstream& rangresSearched, std::fstream& primesFound, std::vector<std::array<unsigned long long, 2>>& primesSearched);

void writePrimesFound(std::fstream& primesFound, std::vector<unsigned long long> primes);

void writeRangesSearched(std::fstream& rangesSearched, std::vector<std::array<unsigned long long, 2>>& primesSearched);

#endif
