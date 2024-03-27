#include "networking/port.hpp"

#include <algorithm>

Port::Port(uint16_t port) : m_port(port)
{
}

Port::Port(std::string port)
{
    if (port.empty())
    {
        throw std::invalid_argument("Port cannot be empty");
    }

    if (std::ranges::any_of(port, [](char c)
                            { return !std::isdigit(c); }))
    {
        throw std::invalid_argument("Port must be a number");
    }

    auto port_int = std::stoi(port);
    if (port_int < PORT_MIN || port_int > PORT_MAX)
    {
        throw std::invalid_argument("Port must be between 0 and 65535");
    }

    m_port = port_int;
}

Port::~Port()
{
}

uint16_t Port::get_port() const
{
    return m_port;
}

std::ostream &operator<<(std::ostream &os, const Port &port)
{
    os << port.get_port();
    return os;
}