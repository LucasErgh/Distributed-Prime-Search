#include "PrimeSearcher.h"

void PrimeSearch::search(){
    inProgress = true;
    for(unsigned long long i = range[0]; i <= range[1]; ++i){
        for (unsigned long long d = 2; d <= i; ++d){
            if (d == i) {
                primes.push_back(i);
                break;
            }
            if (i % d == 0)
                break;
        }
    }
    primesToSend = true;
    inProgress = false;
}

std::vector<unsigned long long> PrimeSearch::getPrimes() { 
    std::vector<unsigned long long> primesCopy = primes;
    primes.clear();
    primesToSend = false; 
    return primesCopy; 
}