#pragma once

#include "networking/socket.hpp"

namespace Socket
{
    class UDP : public Socket
    {
    private:
        Address addr;

    public:
        UDP(const Address &address);
        ~UDP();
    };
}