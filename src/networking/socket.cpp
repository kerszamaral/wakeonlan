#include "networking/socket.hpp"

#include "common/format.hpp"

#ifdef _WIN32
bool Socket::wsaInit = false;
WSADATA Socket::wsaData = WSADATA();
#endif

int Socket::close(socket_t soc)
{
#ifdef _WIN32
    return ::closesocket(soc);
#else
    return ::close(soc);
#endif
}

void Socket::initialize()
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

void Socket::cleanup()
{
#ifdef _WIN32
    if (Socket::getWsaInit())
    {
        WSACleanup();
        Socket::wsaInit = false;
    }
#endif
}

void Socket::error(std::string message)
{
#ifdef _WIN32
    throw std::runtime_error(message + ": " + std::to_string(WSAGetLastError()));
#else
    throw std::runtime_error(message);
#endif
}

void Socket::set_saddr(sockaddr_in &addr, const uint32_t &new_addr)
{
#ifdef _WIN32
    addr.sin_addr.S_un.S_addr = new_addr;
#else
    addr.sin_addr.s_addr = new_addr;
#endif
}

void Socket::checkOpen() const
{
    if (!open)
    {
        throw std::runtime_error("Socket is not open");
    }
}

Socket::Socket(socket_t s)
{
    Socket::initialize();
    sock = s;
    open = true;
}

Socket::Socket(Socket::Type type)
{
    Socket::initialize();
    Protocol prot = (type == Type::TCP) ? Protocol::TCP : Protocol::UDP;
    IPVersion ipver = IPVersion::IPv4;
    sock = ::socket(fmt::to_underlying(ipver), fmt::to_underlying(type), fmt::to_underlying(prot));
    if (sock == Socket::INVALID)
    {
        Socket::error("socket failed");
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
    if (bytes_sent == Socket::ERROR)
    {
        Socket::error("send failed");
    }
}

void Socket::sendto(std::string message, const sockaddr_in &addr) const
{
    checkOpen();
    auto bytes_sent = ::sendto(sock, message.c_str(), message.length(), 0, (sockaddr *)&addr, sizeof(addr));
    if (bytes_sent == Socket::ERROR)
    {
        Socket::error("sendto failed");
    }
}

std::string Socket::receive() const
{
    checkOpen();
    std::string buffer(BUFFER_SIZE, 0);
    auto bytes_received = ::recv(sock, buffer.data(), buffer.length(), 0);
    if (bytes_received == Socket::ERROR)
    {
        Socket::error("recv failed");
    }
    return buffer;
}

void Socket::setOpt(const int &level, const int &optname, const int &optval)
{
    checkOpen();
    auto setsockresult = ::setsockopt(sock, level, optname, (char *)&optval, sizeof(optval));
    if (setsockresult == Socket::ERROR)
    {
        Socket::error("setsockopt failed");
    }
}

void Socket::setNonBlocking(const bool &non_blocking)
{
    checkOpen();
#ifdef _WIN32
    u_long mode = non_blocking ? 1 : 0;
    auto ioctlsocket_result = ::ioctlsocket(sock, FIONBIO, &mode);
    if (ioctlsocket_result == Socket::ERROR)
    {
        Socket::error("ioctlsocket failed");
    }
#else
    auto flags = ::fcntl(sock, F_GETFL, 0);
    if (flags == Socket::ERROR)
    {
        Socket::error("fcntl failed");
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
    if (fcntl_result == Socket::ERROR)
    {
        Socket::error("fcntl failed");
    }
#endif
}

void Socket::bind(const sockaddr_in &addr)
{
    checkOpen();
    auto bind_result = ::bind(sock, (sockaddr *)&addr, sizeof(addr));
    if (bind_result == Socket::ERROR)
    {
        Socket::error("bind failed");
    }
}

void Socket::listen(const int &backlog)
{
    checkOpen();
    auto listen_result = ::listen(sock, backlog);
    if (listen_result == Socket::ERROR)
    {
        Socket::error("listen failed");
    }
}

socket_t Socket::accept(sockaddr_in &addr)
{
    checkOpen();
    auto addr_len = sizeof(struct sockaddr_in);
    socket_t client_socket = ::accept(sock, (sockaddr *)&addr, (socklen_t *)&addr_len);
    if (client_socket == Socket::INVALID)
    {
        Socket::error("accept failed");
    }
    return client_socket;
}

void Socket::connect(const sockaddr_in &addr)
{
    checkOpen();
    auto connect_result = ::connect(sock, (sockaddr *)&addr, sizeof(addr));
    if (connect_result == Socket::ERROR)
    {
        Socket::error("connect failed");
    }
}

void Socket::close()
{
    if (open)
    {
        auto closesocket_result = Socket::close(sock);
        if (closesocket_result == Socket::ERROR)
        {
            Socket::error("closesocket failed");
        }
    }
}
