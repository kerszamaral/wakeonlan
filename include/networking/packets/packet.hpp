#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <variant>
#include <tuple>

#include "common/pcinfo.hpp"
#include "networking/packets/util.hpp"
#include "networking/packets/header.hpp"
#include "networking/packets/body.hpp"

namespace Networking::Packets
{
    class Packet
    {
    private:
        Header header;
        Body body;

    public:
        Packet() noexcept : header(), body() {}
        Packet(const payload_t &data) : header(), body()
        {
            this->deserialize(data);
        }
        Packet(const body_t &payload) : header(), body()
        {
            this->setBody(payload);
        }
        Packet(PacketType type, const body_t &payload) noexcept
        {
            this->body = Body(payload);
            this->header = Header(type, 0, this->body.size(), 0);
        }
        Packet(const Header &header, const Body &body) : header(header), body(body) {}
        Packet(PacketType type, uint16_t seqn, uint16_t timestamp, const payload_t &payload) : header(type, seqn, payload.size(), timestamp), body(payload) {}
        Packet(PacketType type, uint16_t seqn, uint16_t timestamp, const std::string &payload) : header(type, seqn, payload.length(), timestamp), body(payload) {}

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch" // Makes switch exhaustive
        Packet(PacketType type)
        {
            switch (type)
            {
            case PacketType::DATA:
            case PacketType::SSR:
            case PacketType::SSELFIN:
            case PacketType::SSELGT:
                this->body = Body(payload_t());
                break;
            case PacketType::STR:
                this->body = Body(std::string());
                break;
            case PacketType::SSE:
                this->body = Body(PC::getHostname());
                break;
            case PacketType::SSD:
            case PacketType::SSD_ACK:
                this->body = Body(std::make_pair(PC::getHostname(), Addresses::Mac::FromMachine()));
                break;
            case PacketType::MAGIC:
                this->body = Body(Addresses::Mac());
                break;
            case PacketType::SSREP:
                this->body = Body(std::make_pair(0, PC::pc_map_t()));
                break;
            case PacketType::SSR_ACK:
            case PacketType::SSEL:
                this->body = Body(0);
                break;
            }

            this->header = Header(type, 0, this->body.size(), 0);
        }
#pragma GCC diagnostic pop

        payload_t serialize() const
        {
            payload_t data;
            data.reserve(this->size());
            header.serialize(data);
            return body.serialize(data);
        }

        Packet &deserialize(const payload_t &data)
        {
            auto it = header.deserialize(data);
            body.deserialize(payload_t(it, data.end()), header.getType(), header.getLength());
            return *this;
        }

        constexpr PacketType getType() const noexcept { return header.getType(); }
        constexpr Header &getHeader() noexcept { return header; }
        constexpr const Header &getHeader() const noexcept { return header; }
        constexpr Body &getBody() noexcept { return body; }
        constexpr const Body &getBody() const noexcept { return body; }
        size_t size() const noexcept { return header.size() + body.size(); }

        Packet &setBody(const body_t &payload)
        {
            this->body = Body(payload);
            auto &head = this->header;
            auto handle_cases = [&head](auto &&arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>)
                {
                    head = Header(PacketType::STR, 0, arg.size(), 0);
                }
                else if constexpr (std::is_same_v<T, payload_t>)
                {
                    head = Header(PacketType::DATA, 0, arg.size(), 0);
                }
                else if constexpr (std::is_same_v<T, SSE_Data>)
                {
                    head = Header(PacketType::SSD, 0, arg.first.size() + arg.second.size(), 0);
                }
                else if constexpr (std::is_same_v<T, Addresses::Mac>)
                {
                    head = Header(PacketType::MAGIC, 0, 0, 0);
                }
                else if constexpr (std::is_same_v<T, SSREP_Data>)
                {
                    head = Header(PacketType::SSREP, 0, arg.second.size(), 0);
                }
                else if constexpr(std::is_same_v<T, uint32_t>)
                {
                    head = Header(PacketType::SSEL, 0, sizeof(uint32_t), 0);
                }
                else
                {
                    static_assert(always_false_v<T>, "non-exhaustive visitor!");
                }
            };
            std::visit(handle_cases, body.getPayload());
            return *this;
        }

        std::string to_string() const
        {
            return header.to_string() + "\n" + body.to_string();
        }

        friend std::ostream &operator<<(std::ostream &os, const Packet &packet)
        {
            os << packet.to_string();
            return os;
        }
    };
}