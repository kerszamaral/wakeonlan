#pragma once

#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#endif

#include "networking/ipv4.hpp"
#include "networking/port.hpp"

// Parts found in
// https://stackoverflow.com/questions/3509011/socket-programming-in-c
namespace Socket
{
#ifdef _WIN32
    constexpr int ERROR = SOCKET_ERROR;
    constexpr int INVALID = INVALID_SOCKET;
    typedef SOCKET socket_t;
#else
    constexpr int ERROR = -1;
    constexpr int INVALID = -1;
    typedef int socket_t;
#endif

    typedef sockaddr_in addr_t;

    enum class Type : int
    {
        TCP,
        UDP
    };

    enum class SocketType : int
    {
        STREAM = SOCK_STREAM,
        DGRAM = SOCK_DGRAM
    };

    enum class Protocol : int
    {
        TCP = IPPROTO_TCP,
        UDP = IPPROTO_UDP
    };

    enum class IPVersion : int
    {
        IPv4 = AF_INET,
        IPv6 = AF_INET6
    };

    void initialize();
    void cleanup();
    int close(socket_t soc);
    void throw_error(const std::string &message);
    void set_saddr(addr_t &addr, const uint32_t &new_addr);
    uint32_t get_saddr(const addr_t &addr);

    class Address
    {
    private:
        IPv4 ip;
        Port port;
        addr_t addr;

        void init_addr();

    public:
        Address() : ip(), port() { init_addr(); }
        Address(const std::string &address);
        Address(const IPv4 &ip, const Port &port) : ip(ip), port(port) { init_addr(); }
        Address(const std::string &ip, const Port &port) : ip(ip), port(port) { init_addr(); }
        Address(const IPv4 &ip, const uint16_t &port) : ip(ip), port(port) { init_addr(); }
        Address(const std::string &ip, const uint16_t &port) : ip(ip), port(port) { init_addr(); }
        Address(const std::string &ip, const std::string &port) : ip(ip), port(port) { init_addr(); }
        Address(const addr_t &address) : ip(get_saddr(address)), port(address.sin_port), addr(address) {}
        Address(const Address &address) : ip(address.ip), port(address.port), addr(address.addr) {}

        ~Address() {}

        void setIp(const IPv4 &new_ip);
        void setIP(const std::string &new_ip) { setIp(IPv4(new_ip)); }
        void setPort(const Port &new_port);
        void setPort(const uint16_t &new_port) { setPort(Port(new_port)); }
        void setAddr(const addr_t &new_addr);

        IPv4 getIp() const { return ip; }
        Port getPort() const { return port; }
        addr_t getAddr() const { return addr; }
    };

    class Socket
    {
    private:
        constexpr static size_t BUFFER_SIZE = 4096;
        socket_t sock;
        bool open = false;
        void checkOpen() const;

    public:
        // Constructors and destructors
        Socket(socket_t s);
        Socket(Type type);
        ~Socket();

        // Data transfer
        void send(std::string message) const;
        void sendto(std::string message, const Address &addr) const;
        std::string receive() const;

        // Socket options
        void setOpt(const int &level, const int &optname, const int &optval);
        void setNonBlocking(const bool &non_blocking);

        // Getters and setters
        socket_t getSocket() { return sock; }

        // Socket operations
        void bind(const Address &addr);
        void listen(const int &backlog);
        socket_t accept(const Address &addr);
        void connect(const Address &addr);
        void close();

        // Operators
        friend std::ostream &operator<<(std::ostream &os, const Socket &s)
        {
            return os << s.receive();
        }
        friend std::istream &operator>>(std::istream &is, const Socket &s)
        {
            std::string message;
            is >> message;
            s.send(message);
            return is;
        }

// Windows specific
#ifdef _WIN32
    private:
        static WSADATA wsaData;
        static bool wsaInit;

    public:
        static bool getWsaInit() { return wsaInit; }
#endif
    };
}