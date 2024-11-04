#pragma once

#include "config.h"

#include <cstdint>
#include <cstring>
#include <array>
#include <vector>

#define CLOSE_CONNECTION 0x00
#define RANGE 0x01
#define SET 0x02

typedef std::array<unsigned long long, 2> Range;
typedef std::vector<unsigned long long> PList;

// serializes range message into byte array for transmission
std::vector<std::byte> createMsg(const Range& r){
    std::vector<std::byte> msg(19);

    int offset = 0;
    uint8_t msgType = 1;
    uint16_t payloadSize = 2;
    
    // message type
    memcpy(msg.data() + offset, &msgType, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    // message size
    memcpy(msg.data() + offset, &payloadSize, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // range
    memcpy(msg.data() + offset, &r[0], sizeof(r[0]));
    offset += sizeof(r[0]);
    memcpy(msg.data() + offset, &r[1], sizeof(r[1]));

    return msg;
}

// serializes prime list into byte array for transmission 
std::vector<std::byte> createMsg(const PList& primes) {
    const uint8_t msgType = 2;
    const uint16_t payloadSize = primes.size();

    // calculate message size
    size_t size = sizeof(msgType) + sizeof(payloadSize) + payloadSize * sizeof(unsigned long long);

    std::vector<std::byte> msg(size);
    int offset = 0;

    // message type
    std::memcpy(msg.data() + offset, &msgType, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    // payload size
    std::memcpy(msg.data() + offset, &payloadSize, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // list of primes
    for (const auto& cur : primes) {
        std::memcpy(msg.data() + offset, &cur, sizeof(unsigned long long));
        offset += sizeof(unsigned long long);
    }

    return msg;
}

/*
// Convert a integer from base 10 to base 128(for efficient transmition using chars) 
char* B10To128(ull dividend){
    // the number of characters n will be after converted to base be is 
    // cleil (log_b (n)) and with base conversion thats ceil (log(n)/log(b))
    // since we use base two for logs we can just say ceil (log(n)/7)
    int size = ceil(log(dividend)/7);
    char *result = new char[size--];

    // conversion algorithm

    while (dividend > 0){
        result[size--] = (char)(dividend % 128);
        dividend /= 128;
    }

    return result;
}

ull B128To10(char* dividend, int size){
    ull result = 0;

    // this will take the first digit and multiply it by its significance
    // then add it to the result and so on until the first digit * 128^0, itself
    while (size > 0) {
        result += (dividend[--size] * pow(128, size));
    }
    return result;
}

// create message for close_connection or request_range

// create message for set of primes being sent
Range interpreterRange(msg m){
    Range range;
    char* header = m.first, *r = m.second;
}

// create message for range being sent
msg createMessage(std::pair<ull, ull> range){
    char *left = B10To128(range.first), *right = B10To128(range.second);
    
    int lSize = sizeof(left), rSize = sizeof(right), size = lSize + rSize + 1;
    
    // number of bytes in range byte array (both range plus one for space between)
    char *message2 = new char[sizeof(left) + sizeof(right) + 1];

    // copy left range and right range into 
    strcpy(message2, left);
    strcpy((message2+sizeof(left) + 1), right);
    
    // First message always uses default_buflen
    char *message1 = new char[DEFAULT_BUFLEN];

    // this mess should represent a byte array of "<MessageType> <size of left range> <size of right range>"
    message1[0] = 1;
    strcpy((message1 + 2), B10To128(sizeof(left))); 
    strcpy((message1 + 3 + sizeof(B10To128(sizeof(left)))), B10To128(sizeof(right)));
}
*/