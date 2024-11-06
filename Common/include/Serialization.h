#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "config.h"

#include <cstdint>
#include <cstring>
#include <array>
#include <vector>

#define CLOSE_CONNECTION 0x00
#define RANGE 0x01
#define SET 0x02

// Create message for close connection
std::vector<std::byte> createMsg();

std::vector<std::byte> createMsg(const std::vector<unsigned long long>& primes);

// serializes range message into byte array for transmission
std::vector<std::byte> createMsg(const std::array<unsigned long long, 2>& r);

// deserializes header returning an int 
int readMsg(uint8_t *header, uint16_t &payloadSize);

// returns bool indicating success
bool readMsg(std::vector<std::byte>& payload, const uint16_t& payloadSize, std::array<unsigned long long, 2> &range);

// returns bool indicating success
bool readMsg(std::vector<std::byte>& payload, const uint16_t& payloadSize, std::vector<unsigned long long> &primes);

#endif