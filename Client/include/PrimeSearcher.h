#pragma once

#include <vector>
#include <array>


void populateSeive(std::vector<int>& seive){
    seive.push_back(2);
    seive.push_back(3);
    for (int i = 4; i<=1000000;  ++i){
        if (isPrime(i))
            seive.push_back(i);
    }
}

void search(std::array<unsigned long long, 2>& range, std::vector<unsigned long long>& primes);

void search(std::array<unsigned long long, 2>& range, std::vector<unsigned long long>& primes, std::vector<int>& seive);