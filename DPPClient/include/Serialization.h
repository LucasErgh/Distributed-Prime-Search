#pragma once

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

// deserializes header returning an int 
int readMsg(uint8_t *header, uint16_t &payloadSize);

// returns bool indicating success
bool readMsg(std::vector<std::byte>& payload, const uint16_t& payloadSize, std::array<unsigned long long, 2> &range);
