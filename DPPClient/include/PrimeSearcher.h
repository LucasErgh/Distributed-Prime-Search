#pragma once

#include <vector>
#include <array>

class PrimeSearch{
private:
    std::array<unsigned long long, 2> range; 
    std::vector<unsigned long long> primes;

    bool inProgress = false;
    bool primesToSend = false;

public:
    void newRange(std::array<unsigned long long, 2> a){ range = a; }
    std::vector<unsigned long long> getPrimes();
    void search();
};