#include "networking/socket.hpp"

#include "common/format.hpp"

namespace Socket
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
            int iResult = WSAStartup(MAKEWORD(2, 2), &Socket::wsaData);
            if (iResult != 0)
            {
                throw std::runtime_error("WSAStartup failed: " + std::to_string(iResult));
            }
            Socket::wsaInit = true;
        }
#endif
    }

    void cleanup()
    {
#ifdef _WIN32
        if (Socket::getWsaInit())
        {
            WSACleanup();
            Socket::wsaInit = false;
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

    void set_saddr(addr_t &addr, const uint32_t &new_addr)
    {
#ifdef _WIN32
        addr.sin_addr.S_un.S_addr = new_addr;
#else
        addr.sin_addr.s_addr = new_addr;
#endif
    }

    uint32_t get_saddr(const addr_t &addr)
    {
#ifdef _WIN32
        return addr.sin_addr.S_un.S_addr;
#else
        return addr.sin_addr.s_addr;
#endif
    }

    void Address::init_addr()
    {
        addr.sin_family = fmt::to_underlying(IPVersion::IPv4);
        addr.sin_port = port.getPort();
        set_saddr(addr, ip.to_network_order());
    }

    Address::Address(const std::string &address)
    {
        auto colon = address.find(':');
        if (colon == std::string::npos)
        {
            throw std::runtime_error("Invalid address");
        }
        ip = IPv4(address.substr(0, colon));
        port = Port(address.substr(colon + 1));
        init_addr();
    }

    void Address::setIp(const IPv4 &new_ip)
    {
        ip = new_ip;
        set_saddr(addr, ip.to_network_order());
    }

    void Address::setPort(const Port &new_port)
    {
        port = new_port;
        addr.sin_port = port.getPort();
    }

    void Address::setAddr(const addr_t &new_addr)
    {
        addr = new_addr;
        ip = IPv4(get_saddr(addr));
        port = Port(addr.sin_port);
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
        const SocketType stype = (type == Type::TCP) ? SocketType::STREAM : SocketType::DGRAM;
        const Protocol prot = (type == Type::TCP) ? Protocol::TCP : Protocol::UDP;
        const IPVersion ipver = IPVersion::IPv4;
        sock = ::socket(fmt::to_underlying(ipver), fmt::to_underlying(stype), fmt::to_underlying(prot));
        if (sock == INVALID)
        {
            throw_error("socket failed");
        }
        open = true;
    }

    Socket::~Socket()
    {
    }

    void Socket::send(std::string message) const
    {
        checkOpen();
        auto bytes_sent = ::send(sock, message.c_str(), message.length(), 0);
        if (bytes_sent == ERROR)
        {
            throw_error("send failed");
        }
    }

    void Socket::sendto(std::string message, const Address &addr) const
    {
        checkOpen();
        const auto &address = addr.getAddr();
        const auto bytes_sent = ::sendto(sock, message.c_str(), message.length(), 0, (sockaddr *)&address, sizeof(address));
        if (bytes_sent == ERROR)
        {
            throw_error("sendto failed");
        }
    }

    std::string Socket::receive() const
    {
        checkOpen();
        std::string buffer(BUFFER_SIZE, 0);
        auto bytes_received = ::recv(sock, buffer.data(), buffer.length(), 0);
        if (bytes_received == ERROR)
        {
            throw_error("recv failed");
        }
        return buffer;
    }

    void Socket::setOpt(const int &level, const int &optname, const int &optval)
    {
        checkOpen();
        auto setsockresult = ::setsockopt(sock, level, optname, (char *)&optval, sizeof(optval));
        if (setsockresult == ERROR)
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
        if (ioctlsocket_result == ERROR)
        {
            error("ioctlsocket failed");
        }
#else
        auto flags = ::fcntl(sock, F_GETFL, 0);
        if (flags == ERROR)
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
        if (fcntl_result == ERROR)
        {
            throw_error("fcntl failed");
        }
#endif
    }

    void Socket::bind(const Address &addr)
    {
        checkOpen();
        const auto &address = addr.getAddr();
        auto bind_result = ::bind(sock, (sockaddr *)&address, sizeof(address));
        if (bind_result == ERROR)
        {
            throw_error("bind failed");
        }
    }

    void Socket::listen(const int &backlog)
    {
        checkOpen();
        auto listen_result = ::listen(sock, backlog);
        if (listen_result == ERROR)
        {
            throw_error("listen failed");
        }
    }

    socket_t Socket::accept(const Address &addr)
    {
        checkOpen();
        const auto &address = addr.getAddr();
        auto addr_len = sizeof(addr_t);
        socket_t client_socket = ::accept(sock, (sockaddr *)&address, (socklen_t *)&addr_len);
        if (client_socket == INVALID)
        {
            throw_error("accept failed");
        }
        return client_socket;
    }

    void Socket::connect(const Address &addr)
    {

        checkOpen();
        const auto &address = addr.getAddr();
        auto connect_result = ::connect(sock, (sockaddr *)&address, sizeof(address));
        if (connect_result == ERROR)
        {
            throw_error("connect failed");
        }
    }

    void Socket::close()
    {
        if (open)
        {
            auto closesocket_result = ::Socket::close(sock);
            if (closesocket_result == ERROR)
            {
                throw_error("closesocket failed");
            }
        }
    }
}