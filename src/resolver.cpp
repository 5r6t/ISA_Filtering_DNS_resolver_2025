/**
 * @file resolver.cpp
 * @brief Hostname/IP resolution
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#include "resolver.h"
#include "common.h"
#include "errors.h"

#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> // getaddrinfo

/// @brief Resolves a hostname or IP string to a sockaddr_storage.
/// @param host_name Hostname or IP address to resolve.
/// @param port      Port to set in the resulting address.
/// @return sockaddr_storage containing the resolved address.
/// @note When printing resolved addresses in dual-stack setups, trim IPv6 scope/formatting as needed.
sockaddr_storage resolve_host(const std::string &host_name, uint16_t port) {

    if(host_name.empty()) {
        std::cerr << "Error: Hostname/IP address is empty.\n";
        exit(ERR_BAD_INPUT);
    }
    
    struct addrinfo hints{};
    struct addrinfo *result = nullptr; // list of addrrs and result of getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP

    int ret = getaddrinfo(host_name.c_str(), nullptr, &hints, &result);
    if (ret != 0) {
        std::cerr << "ERROR: Unable to resolve domain name: " << host_name << "\n";
        exit(ERR_INTERNAL);
    }

    struct sockaddr_storage addr{};
    int family = AF_UNSPEC;

    // first address found is kept
    for (auto next = result; next != nullptr; next = next->ai_next) {
        if (next->ai_family == AF_INET) {
            addr = *reinterpret_cast<sockaddr_storage*>(next->ai_addr);
            reinterpret_cast<sockaddr_in*>(&addr)->sin_port = htons(port);
            family = AF_INET;
            break; // found address
        } else if (next->ai_family == AF_INET6) {
            addr = *reinterpret_cast<sockaddr_storage*>(next->ai_addr);
            reinterpret_cast<sockaddr_in6*>(&addr)->sin6_port = htons(port);
            family = AF_INET6;
            break; // found address
        }
    }
    if (family == AF_UNSPEC) {
        std::cerr << "ERROR: No valid IPv4/IPv6 address found for: " << host_name << "\n";
        freeaddrinfo(result);
        exit(ERR_INTERNAL);
    }

    // debug
    char buf[INET6_ADDRSTRLEN];
    void *src = (family == AF_INET) 
       ? reinterpret_cast<void*>(&reinterpret_cast<sockaddr_in*>(&addr)->sin_addr)
       : reinterpret_cast<void*>(&reinterpret_cast<sockaddr_in6*>(&addr)->sin6_addr);
    inet_ntop(family, src, buf, sizeof(buf));
    printf_debug("Resolved %s -> %s:%d", host_name.c_str(), buf, port);
    
    freeaddrinfo(result);

    return addr; // return address 
}