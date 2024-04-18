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
        constexpr Packet() noexcept : header(), body() {}
        constexpr Packet(const payload_t &data) : header(), body()
        {
            this->deserialize(data);
        }
        constexpr Packet(const body_t &payload) : header(), body()
        {
            this->setBody(payload);
        }
        constexpr Packet(PacketType type, const body_t &payload) noexcept
        {
            this->body = Body(payload);
            this->header = Header(type, 0, this->body.size(), 0);
        }
        constexpr Packet(const Header &header, const Body &body) : header(header), body(body) {}
        constexpr Packet(PacketType type, uint16_t seqn, uint16_t timestamp, const payload_t &payload) : header(type, seqn, payload.size(), timestamp), body(payload) {}
        constexpr Packet(PacketType type, uint16_t seqn, uint16_t timestamp, const std::string &payload) : header(type, seqn, payload.length(), timestamp), body(payload) {}

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch" // Makes switch exhaustive
        constexpr Packet(PacketType type)
        {
            switch (type)
            {
            case PacketType::DATA:
            case PacketType::SSR:
            case PacketType::SSR_ACK:
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
                this->body = Body(std::make_pair(PC::getHostname(), Addresses::Mac::FromMachine().value()));
                break;
            case PacketType::MAGIC:
                this->body = Body(Addresses::Mac());
                break;
            }

            this->header = Header(type, 0, this->body.size(), 0);
        }
#pragma GCC diagnostic pop

        constexpr payload_t serialize() const
        {
            payload_t data;
            data.reserve(this->size());
            header.serialize(data);
            return body.serialize(data);
        }

        constexpr Packet &deserialize(const payload_t &data)
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
        constexpr size_t size() const noexcept { return header.size() + body.size(); }

        constexpr Packet &setBody(const body_t &payload)
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