#include "Serialization.h"

std::vector<std::byte> createMsg(const std::array<unsigned long long, 2>& r){
    std::vector<std::byte> msg(19);

    int offset = 0;
    uint8_t msgType = 0x01;
    uint16_t payloadSize = 2;
    
    // message type
    memcpy(msg.data() + offset, &msgType, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    // message size
    memcpy(msg.data() + offset, &payloadSize, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // range
    memcpy(msg.data() + offset, &r[0], sizeof(unsigned long long));
    offset += sizeof(r[0]);
    memcpy(msg.data() + offset, &r[1], sizeof(unsigned long long));

    return msg;
}

// deserializes header returning an int 
int readMsg(uint8_t *header, uint16_t &payloadSize){
    uint8_t msgType = header[0];
    memcpy(&payloadSize, reinterpret_cast<char*>(header+1), sizeof(uint16_t));
    return msgType;
}

// returns bool indicating success
bool readMsg(std::vector<std::byte>& payload, const uint16_t& payloadSize, std::vector<unsigned long long> &primes){
    
    for(size_t offset = 0; offset < payloadSize; offset++){
        unsigned long long val;
        memcpy(&val, payload.data() + offset*8, sizeof(unsigned long long));
        primes.at(offset) = val;
    }
    // To-Do verify list of primes, for now we just return true
    return true; 
}