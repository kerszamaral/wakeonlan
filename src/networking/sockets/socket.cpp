#include "networking/sockets/socket.hpp"

#include "common/format.hpp"
#include <cstring>

namespace Networking::Sockets
{
#ifdef OS_WIN
    bool Socket::wsaInit = false;
    WSADATA Socket::wsaData = WSADATA();
#endif

    int close(socket_t soc)
    {
#ifdef OS_WIN
        return ::closesocket(soc);
#else
        return ::close(soc);
#endif
    }

    void initialize()
    {
#ifdef OS_WIN
        if (!Socket::getWsaInit())
        {
            int iResult = WSAStartup(MAKEWORD(2, 2), Socket::getWsaData());
            if (iResult != 0)
            {
                throw std::runtime_error("WSAStartup failed: " + std::to_string(iResult));
            }
            Socket::setWsaInit(true);
        }
#endif
    }

    void cleanup()
    {
#ifdef OS_WIN
        if (Socket::getWsaInit())
        {
            WSACleanup();
            Socket::setWsaInit(false);
        }
#endif
    }

    std::runtime_error socket_error(const std::string &message)
    {
#ifdef OS_WIN
        return std::runtime_error(message + ": " + std::to_string(WSAGetLastError()));
#else
        return std::runtime_error(message);
#endif
    }

    success_t Socket::checkOpen() const
    {
#ifdef OS_WIN
        if (!Socket::getWsaInit())
        {
            return false;
        }
#endif
        if (!getOpen())
        {
            return false;
        }
        return true;
    }

    Socket::Socket(socket_t s)
    {
        initialize();
        sock = s;
    }

    Socket::Socket(Type type)
    {
        initialize();
        this->type = type;
        const auto stype = (type == Type::TCP) ? SocketType::STREAM : SocketType::DGRAM;
        const auto prot = (type == Type::TCP) ? Protocol::TCP : Protocol::UDP;
        const auto ipver = Networking::Addresses::IPVersion::IPv4;
        sock = ::socket(fmt::to_underlying(ipver), fmt::to_underlying(stype), fmt::to_underlying(prot));
        if (sock == SOCK_INVALID)
        {
            throw socket_error("socket failed");
        }
    }

    Socket::~Socket()
    {
    }

    std::optional<std::reference_wrapper<Socket>> Socket::setOpt(const int &level, const int &optname, const int &optval)
    {
        if (!checkOpen())
            return std::nullopt;
        auto setsockresult = ::setsockopt(sock, level, optname, (char *)&optval, sizeof(optval));
        if (setsockresult == SOCK_ERROR)
        {
            return std::nullopt;
        }
        return *this;
    }

    std::optional<std::reference_wrapper<Socket>> Socket::setNonBlocking(const bool &non_blocking)
    {
        if (!checkOpen())
            return std::nullopt;
#ifdef OS_WIN
        u_long mode = non_blocking ? 1 : 0;
        auto ioctlsocket_result = ::ioctlsocket(sock, FIONBIO, &mode);
        if (ioctlsocket_result == SOCK_ERROR)
        {
            return std::nullopt;
        }
#else
        auto flags = ::fcntl(sock, F_GETFL, 0);
        if (flags == SOCK_ERROR)
        {
            return std::nullopt;
        }
        if (non_blocking)
        {
            flags |= O_NONBLOCK;
        }
        else
        {
            flags &= ~O_NONBLOCK;
        }
        auto fcntl_result = ::fcntl(sock, F_SETFL, flags);
        if (fcntl_result == SOCK_ERROR)
        {
            return std::nullopt;
        }
#endif
        this->non_blocking = non_blocking;
        return *this;
    }

    std::optional<std::reference_wrapper<Socket>> Socket::bind(const Networking::Addresses::Address &addr)
    {
        if (!checkOpen())
            return std::nullopt;
        const auto &address = addr.getAddr();
        auto bind_result = ::bind(sock, (sockaddr *)&address, sizeof(address));
        if (bind_result == SOCK_ERROR)
        {
            return std::nullopt;
        }
        bound = true;
        return *this;
    }

    std::optional<std::reference_wrapper<Socket>> Socket::listen(const int &backlog)
    {
        if (!checkOpen())
            return std::nullopt;
        auto listen_result = ::listen(sock, backlog);
        if (listen_result == SOCK_ERROR)
        {
            return std::nullopt;
        }
        return *this;
    }

    std::optional<std::pair<Socket, Networking::Addresses::Address>> Socket::accept()
    {
        if (!checkOpen())
            return std::nullopt;
        Networking::Addresses::addr_t addr;
        auto addr_len = sizeof(addr);
        socket_t client_socket = ::accept(sock, (sockaddr *)&addr, (socklen_t *)&addr_len);
        if (client_socket == SOCK_INVALID)
        {
            return std::nullopt;
        }
        return std::make_pair(Socket(client_socket), Networking::Addresses::Address(addr));
    }

    std::optional<std::reference_wrapper<Socket>> Socket::connect(const Networking::Addresses::Address &addr)
    {
        if (!checkOpen())
            return std::nullopt;
        const auto &address = addr.getAddr();
        auto connect_result = ::connect(sock, (sockaddr *)&address, sizeof(address));
        if (connect_result == SOCK_ERROR)
        {
            return std::nullopt;
        }
        return *this;
    }

    success_t Socket::close()
    {
        if (!checkOpen())
        {
            auto closesocket_result = Sockets::close(sock);
            if (closesocket_result == SOCK_ERROR)
            {
                return false;
            }
        }
        sock = SOCK_INVALID;
        return true;
    }
}