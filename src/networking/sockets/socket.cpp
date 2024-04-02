#include "networking/sockets/socket.hpp"

#include "common/format.hpp"

namespace Networking::Sockets
{
#ifdef _WIN32
    bool Socket::wsaInit = false;
    WSADATA Socket::wsaData = WSADATA();
#endif

    int close(socket_t soc)
    {
#ifdef _WIN32
        return ::closesocket(soc);
#else
        return ::close(soc);
#endif
    }

    void initialize()
    {
#ifdef _WIN32
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
#ifdef _WIN32
        if (Socket::getWsaInit())
        {
            WSACleanup();
            Socket::setWsaInit(false);
        }
#endif
    }

    void throw_error(const std::string &message)
    {
#ifdef _WIN32
        throw std::runtime_error(message + ": " + std::to_string(WSAGetLastError()));
#else
        throw std::runtime_error(message);
#endif
    }

    void Socket::checkOpen() const
    {
#ifdef _WIN32
        if (!wsaInit)
        {
            throw std::runtime_error("WSAStartup not called");
        }
#endif
        if (!open)
        {
            throw std::runtime_error("Socket is not open");
        }
    }

    Socket::Socket(socket_t s)
    {
        initialize();
        sock = s;
        open = true;
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
            throw_error("socket failed");
        }
        open = true;
    }

    Socket::~Socket()
    {
    }

    void Socket::setOpt(const int &level, const int &optname, const int &optval)
    {
        checkOpen();
        auto setsockresult = ::setsockopt(sock, level, optname, (char *)&optval, sizeof(optval));
        if (setsockresult == SOCK_ERROR)
        {
            throw_error("setsockopt failed");
        }
    }

    void Socket::setNonBlocking(const bool &non_blocking)
    {
        checkOpen();
#ifdef _WIN32
        u_long mode = non_blocking ? 1 : 0;
        auto ioctlsocket_result = ::ioctlsocket(sock, FIONBIO, &mode);
        if (ioctlsocket_result == SOCK_ERROR)
        {
            throw_error("ioctlsocket failed");
        }
#else
        auto flags = ::fcntl(sock, F_GETFL, 0);
        if (flags == SOCK_ERROR)
        {
            throw_error("fcntl failed");
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
            throw_error("fcntl failed");
        }
#endif
        this->non_blocking = non_blocking;
    }

    void Socket::bind(const Networking::Addresses::Address &addr)
    {
        checkOpen();
        const auto &address = addr.getAddr();
        auto bind_result = ::bind(sock, (sockaddr *)&address, sizeof(address));
        if (bind_result == SOCK_ERROR)
        {
            throw_error("bind failed");
        }
        bound = true;
    }

    void Socket::listen(const int &backlog)
    {
        checkOpen();
        auto listen_result = ::listen(sock, backlog);
        if (listen_result == SOCK_ERROR)
        {
            throw_error("listen failed");
        }
    }

    std::pair<Socket, Networking::Addresses::Address> Socket::accept()
    {
        checkOpen();
        Networking::Addresses::addr_t addr;
        auto addr_len = sizeof(addr);
        socket_t client_socket = ::accept(sock, (sockaddr *)&addr, (socklen_t *)&addr_len);
        if (client_socket == SOCK_INVALID)
        {
            throw_error("accept failed");
        }
        return std::make_pair(Socket(client_socket), Networking::Addresses::Address(addr));
    }

    void Socket::connect(const Networking::Addresses::Address &addr)
    {

        checkOpen();
        const auto &address = addr.getAddr();
        auto connect_result = ::connect(sock, (sockaddr *)&address, sizeof(address));
        if (connect_result == SOCK_ERROR)
        {
            throw_error("connect failed");
        }
    }

    void Socket::close()
    {
        if (open)
        {
            auto closesocket_result = Sockets::close(sock);
            if (closesocket_result == SOCK_ERROR)
            {
                throw_error("closesocket failed");
            }
        }
    }
}