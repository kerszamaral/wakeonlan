#pragma once

#include <iostream>

#include "networking/networking.hpp"

#include "networking/addresses/address.hpp"

// Parts found in
// https://stackoverflow.com/questions/3509011/socket-programming-in-c
namespace Networking::Sockets
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

    enum class Type : int
    {
        TCP,
        UDP
    };

    enum class Mode : int
    {
        SERVER,
        CLIENT
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

    void initialize();
    void cleanup();
    int close(socket_t soc);
    void throw_error(const std::string &message);

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
        void sendto(std::string message, const Networking::Addresses::Address &addr) const;
        std::string receive() const;

        // Socket options
        void setOpt(const int &level, const int &optname, const int &optval);
        void setNonBlocking(const bool &non_blocking);

        // Getters and setters
        socket_t getSocket() { return sock; }

        // Socket operations
        void bind(const Networking::Addresses::Address &addr);
        void listen(const int &backlog);
        socket_t accept(const Networking::Addresses::Address &addr);
        void connect(const Networking::Addresses::Address &addr);
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