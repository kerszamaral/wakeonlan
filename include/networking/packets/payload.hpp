#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <variant>
#include <tuple>

#include "common/pcinfo.hpp"
#include "networking/addresses/mac.hpp"

namespace Networking::Packets
{
    enum class PacketType : uint16_t
    {
        DATA = 0x0000,
        STR = 0x0001,
        SSD = 0x0002,     // Sleep Service Discovery
        SSD_ACK = 0x0003, // Sleep Service Discovery Acknowledgement
        SSE = 0x0004,     // Sleep Service Exit
        MAGIC = 0xFFFF,   // Magic Packet
        // DATA = 0x0001,
        // CMD = 0x0002,
        // ACK = 0x0003,
        // NACK = 0x0004,
    };

    typedef std::vector<uint8_t> payload_t;

    typedef std::pair<PC::hostname_t, Addresses::Mac> SSE_Data;

    typedef std::variant<std::string, payload_t, SSE_Data, Addresses::Mac> body_t;

    constexpr inline void extendBytes(payload_t &vec, const uint8_t *data, size_t size)
    {
        vec.insert(vec.end(), data, data + size);
    }

    constexpr inline void extendBytes(payload_t &vec, const uint16_t &data)
    {
        vec.push_back(data & 0xFF);
        vec.push_back((data >> 8) & 0xFF);
    }
}