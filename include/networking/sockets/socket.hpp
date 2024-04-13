#pragma once

#include <iostream>
#include <tuple>
#include "common/optional.hpp"
#include <functional>

#include "networking/networking.hpp"

#include "networking/addresses/address.hpp"
#include "networking/macaddr.hpp"

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
    std::runtime_error socket_error(const std::string &message);
    typedef bool success_t;

    class Socket
    {
    private:
        socket_t sock = SOCK_INVALID;
        Type type;
        bool non_blocking = false;
        bool bound = false;

    protected:
        success_t checkOpen() const;
        constexpr static size_t BUFFER_SIZE = 4096;

    public:
        // Constructors and destructors
        Socket(socket_t s);
        Socket(Type type);
        ~Socket();

        // Socket options
        opt::optional<std::reference_wrapper<Socket>> setOpt(const int &level, const int &optname, const int &optval);
        opt::optional<std::reference_wrapper<Socket>> setNonBlocking(const bool &non_blocking);

        // Getters and setters
        socket_t getSocket() const { return sock; }
        Type getType() const { return type; }
        bool getOpen() const { return sock != SOCK_INVALID; }
        bool getNonBlocking() const { return non_blocking; }
        bool getBound() const { return bound; }
        static opt::optional<Networking::MacAddress> getMacAddr(const std::string &intrfc);
        static opt::optional<Networking::MacAddress> getMacAddr() { return getMacAddr("eth0"); }

        // Socket operations

        opt::optional<std::reference_wrapper<Socket>> bind(const Networking::Addresses::Address &addr);
        opt::optional<std::reference_wrapper<Socket>> listen(const int &backlog);
        opt::optional<std::pair<Socket, Networking::Addresses::Address>> accept();
        opt::optional<std::reference_wrapper<Socket>> connect(const Networking::Addresses::Address &addr);
        success_t close();

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