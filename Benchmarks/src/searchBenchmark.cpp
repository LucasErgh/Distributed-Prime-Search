/*
    searchBenchmark.cpp
*/

#include <iostream>
#include <chrono>
#include "PrimeSearcher.h"

int main(){
    using namespace std::chrono;
    PrimeSearch search;
    const unsigned long long upperRange = 100000;
    const auto startTime = system_clock::now();
    search.newRange({2, upperRange});
    search.search();
    auto duration = duration_cast<milliseconds>(system_clock::now() - startTime).count();

    std::cout << "Searching [2, " << upperRange << "] duration: " << duration << " miliseconds\n";
}