/**
 * @file udp.cpp
 * @brief UDP socket creation, binding, and packet send/receive operations.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#include "udp.h"
#include "common.h"
#include "errors.h"

#include <iostream>
#include <cstring>
#include <cstdint>
#include <initializer_list>
#include <vector>

#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

constexpr int DNS_UDP_MAX_B = 512;

/// @brief Closes each socket in the list if open.
/// @param socks List of socket pointers to close.
void sock_close(std::initializer_list<int*> socks) {
    for (int* sock : socks) {
        if (sock && *sock != -1) {
            close(*sock);
            *sock = -1;
        }
    }
}

/// @brief Creates an UDP socket for the given address family.
/// @param family AF_INET or AF_INET6.
/// @return File descriptor of the created socket.
int create_udp_socket(int family) 
{
    int sock = socket(family, SOCK_DGRAM, 0);
    
    if (sock == -1) {
        std::cerr << "ERROR: Cannot create socket.\n";
        exit(ERR_INTERNAL);
    }

    // configure socket options (reuseaddr, reuseport)
    int enable = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));

    if (family == AF_INET6) {
        int off = 0; // allow both v4 and v6
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof(off));
    }
    
    return sock;
}

/// @brief Binds a UDP socket to the specified port.
/// @param sock   Socket file descriptor.
/// @param port   Port to bind.
/// @param family Address family (AF_INET or AF_INET6).
void bind_udp_socket(int sock, int port, int family)
{
    if (family == AF_INET) {
        struct sockaddr_in s_addr{};

        s_addr.sin_family = AF_INET; 
        s_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
        s_addr.sin_port = htons(port); 

        int addr_size = sizeof(s_addr); 
        const sockaddr* addr = reinterpret_cast<sockaddr*>(&s_addr);

        if (bind(sock, addr, addr_size) < 0) 
        { 
            std::cerr << "ERROR: Cannot bind IPv4 socket.\n";
            exit(ERR_SOCK_BIND);
        }
    } 
    else if (family == AF_INET6) {
        struct sockaddr_in6 s_addr6{};

        s_addr6.sin6_family = AF_INET6; 
        s_addr6.sin6_addr = in6addr_any; 
        s_addr6.sin6_port = htons(port);

        int addr6_size = sizeof(s_addr6);
        const sockaddr* addr6 = reinterpret_cast<sockaddr*>(&s_addr6);

        if (bind(sock, addr6, addr6_size) < 0) 
        { 
            std::cerr << "ERROR: Cannot bind IPv6 socket.\n";
            exit(ERR_SOCK_BIND);
        }
    }
    else {
        std::cerr << "ERROR: Unsupported address family for binding.\n";
        exit(ERR_INTERNAL);
    }
    printf_debug("Socket bound to port %d (family %s)", port,
                    (family == AF_INET ? "IPv4" : "IPv6"));
}

/// @brief Receives a UDP packet from the socket.
/// @param sock Socket file descriptor.
/// @return UdpPacket containing data and source address.
UdpPacket udp_receive (const int sock) {
    UdpPacket pkt{};
    pkt.data.resize(DNS_UDP_MAX_B);
    pkt.len = sizeof(pkt.src);
    
    ssize_t bytes_rx = recvfrom(sock, pkt.data.data(), pkt.data.size(), 0, 
                                (sockaddr*)&pkt.src, &pkt.len); // help 

    if (bytes_rx < 0) 
    {
        perror("ERROR: recvfrom");
        return {};
    }

    pkt.data.resize(bytes_rx); // shrink vector to packet length
    printf_debug("Received %zd bytes via UDP.", bytes_rx);
    return pkt;
}

/// @brief Sends a UDP packet to the specified address.
/// @param sock Socket file descriptor.
/// @param data Payload to send.
/// @param addr Destination address.
void udp_send (const int sock, const std::vector<uint8_t>& data, const sockaddr_storage& addr) {
    
    socklen_t addrlen = 0;

    if (addr.ss_family == AF_INET)
        addrlen = sizeof(struct sockaddr_in);
    else if (addr.ss_family == AF_INET6)
        addrlen = sizeof(struct sockaddr_in6);
    else {
        printf_debug("ERROR: Unsupported address family %d", addr.ss_family);
        return;
    }

    int flags = 0;

    ssize_t bytes_tx = sendto(sock, data.data(), data.size(), flags, (sockaddr*)&addr, addrlen);
    
    if (bytes_tx < 0) {
        perror("ERROR: sendto");
    }

    printf_debug("Sent %zd bytes via UDP", bytes_tx);
}