#pragma once

#include <cstdint>
#include <cstring>
#include <array>
#include <vector>

#define CLOSE_CONNECTION 0x00
#define RANGE 0x01
#define SET 0x02

typedef std::array<unsigned long long, 2> Range;
typedef std::vector<unsigned long long> PList;

// To-Do add deserialization function

// serializes prime list into byte array for transmission 
std::vector<std::byte> createMsg(const PList& primes) {
    const uint8_t msgType = SET;
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