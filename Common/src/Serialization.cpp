#include "Serialization.h"

// Create message for close connection
std::vector<std::byte> createMsg(){
    const uint8_t msgType = CLOSE_CONNECTION;
    const uint16_t payloadSize = 0;

    size_t offset = 0;

    std::vector<std::byte> msg(3);
    std::memcpy(msg.data() + offset++, &msgType, sizeof(uint8_t));
    std::memcpy(msg.data() + offset, &payloadSize, sizeof(payloadSize));
    return msg;
}

std::vector<std::byte> createMsg(const std::vector<unsigned long long>& primes) {
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

// deserializes header returning message type as an int
int readMsg(uint8_t *header, uint16_t &payloadSize){
    uint8_t msgType = header[0];
    memcpy(&payloadSize, reinterpret_cast<char*>(header+1), sizeof(uint16_t));
    return msgType;
}

// returns bool indicating success
bool readMsg(std::vector<std::byte>& payload, const uint16_t& payloadSize, std::array<unsigned long long, 2> &range){
    int pos = 0;
    for(size_t offset = 0; offset < payloadSize * sizeof(unsigned long long); offset += sizeof(unsigned long long)){
        unsigned long long val = 0;
        memcpy(&val, reinterpret_cast<char*>(payload.data()+offset), sizeof(unsigned long long));   
        range[pos++] = val;
    }
    // To-Do verify numbers given
    return true;
}

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
