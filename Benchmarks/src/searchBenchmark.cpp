/*
    searchBenchmark.cpp
*/

#include <iostream>
#include <chrono>
#include "PrimeSearcher.h"

int main(){
    using namespace std::chrono;

    std::array<unsigned long long, 2> range = {2, 10000000};
    std::vector<unsigned long long> primes;

    const auto startTime = system_clock::now();
    search(range, primes);
    auto duration = duration_cast<milliseconds>(system_clock::now() - startTime).count();

    std::cout << "Searching [2, 10000000] duration: " << duration << " miliseconds\n";
}