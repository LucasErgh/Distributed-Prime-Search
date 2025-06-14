#include "PrimeSearcher.h"

bool isPrime(unsigned long long num){
    for (unsigned long long d = 2; d*d <= num; ++d){
        if (num % d == 0)
            return false;
    }
    return true;
}

void search(std::array<unsigned long long, 2>& range, std::vector<unsigned long long>& primes) {
    unsigned long long i;
    if ((range[0] - 1) % 6 == 0){
        if (isPrime(range[0])){
            primes.push_back(range[0]);
        }
        i = range[0] + 5;
    } else {
        i = range[0];
        while (i % 6 != 0)
            ++i;
    }
    for(i; i <= range[1]; i+=6 ){
        if (isPrime(i-1))
            primes.push_back(i-1);
        if (isPrime(i+1))
            primes.push_back(i+1);
    }
}
