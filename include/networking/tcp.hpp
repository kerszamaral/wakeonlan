#pragma once

#include "networking/socket.hpp"

namespace Socket
{
    class TCP : public Socket
    {
    public:
        TCP(socket_t s) : Socket(s) {}
        TCP(const Address &addr);
        ~TCP();
    };
}