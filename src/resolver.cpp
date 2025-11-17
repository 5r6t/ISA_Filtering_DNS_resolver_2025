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

/// @brief Convert a sockaddr_storage address to a readable string.
/// @param addr Socket address (IPv4 or IPv6).
/// @return Pointer to a static null-terminated string containing the
///         textual IP representation, or "UNKNOWN" for unsupported families.
/// @note The returned buffer is static and will be overwritten by subsequent calls.
const char* addr_to_str(const sockaddr_storage& addr) {
    static char buf[INET6_ADDRSTRLEN];

    int family = addr.ss_family;
    const void* src = nullptr;

    if (family == AF_INET) {
        src = &reinterpret_cast<const sockaddr_in&>(addr).sin_addr;
    } else if (family == AF_INET6) {
        src = &reinterpret_cast<const sockaddr_in6&>(addr).sin6_addr;
    } else {
        return "UNKNOWN";
    }

    inet_ntop(family, src, buf, sizeof(buf));
    return buf;
}


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
        std::memcpy(&addr, next->ai_addr, next->ai_addrlen);
        if (next->ai_family == AF_INET) {
            reinterpret_cast<sockaddr_in*>(&addr)->sin_port = htons(port);
            family = AF_INET;
            break;
        } else if (next->ai_family == AF_INET6) {
            reinterpret_cast<sockaddr_in6*>(&addr)->sin6_port = htons(port);
            family = AF_INET6;
            break;
        }
    }
    if (family == AF_UNSPEC) {
        std::cerr << "ERROR: No valid IPv4/IPv6 address found for: " << host_name << "\n";
        freeaddrinfo(result);
        exit(ERR_INTERNAL);
    }

    // debug


    printf_debug("Resolved %s -> %s:%d", host_name.c_str(), addr_to_str(addr), port);
    
    freeaddrinfo(result);

    return addr; // return address 
}