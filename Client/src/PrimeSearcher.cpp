#include "PrimeSearcher.h"

void search(std::array<unsigned long long, 2>& range, std::vector<unsigned long long>& primes) {
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
}
