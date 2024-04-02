#pragma once

#include <iostream>
#include <tuple>

#include "networking/networking.hpp"

#include "networking/addresses/address.hpp"

// Parts found in
// https://stackoverflow.com/questions/3509011/socket-programming-in-c
namespace Networking::Sockets
{
#ifdef _WIN32
    constexpr int SOCK_ERROR = int(SOCKET_ERROR);
    constexpr size_t SOCK_INVALID = INVALID_SOCKET;
    typedef SOCKET socket_t;
#else
    constexpr int SOCK_ERROR = -1;
    constexpr int SOCK_INVALID = -1;
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
        socket_t sock;
        Type type;
        bool open = false;
        bool non_blocking = false;
        bool bound = false;

    protected:
        void checkOpen() const;
        constexpr static size_t BUFFER_SIZE = 4096;

    public:
        // Constructors and destructors
        Socket(socket_t s);
        Socket(Type type);
        ~Socket();

        // Socket options
        void setOpt(const int &level, const int &optname, const int &optval);
        void setNonBlocking(const bool &non_blocking);

        // Getters and setters
        socket_t getSocket() const { return sock; }
        Type getType() const { return type; }
        bool getOpen() const { return open; }
        bool getNonBlocking() const { return non_blocking; }
        bool getBound() const { return bound; }

        // Socket operations
        void bind(const Networking::Addresses::Address &addr);
        void listen(const int &backlog);
        std::pair<Socket, Networking::Addresses::Address> accept();
        void connect(const Networking::Addresses::Address &addr);
        void close();

// Windows specific
#ifdef _WIN32
    private:
        static WSADATA wsaData;
        static bool wsaInit;

    public:
        static void setWsaInit(bool init) { wsaInit = init; }
        static bool getWsaInit() { return wsaInit; }
        static WSADATA *getWsaData() { return &wsaData; }
#endif
    };
}