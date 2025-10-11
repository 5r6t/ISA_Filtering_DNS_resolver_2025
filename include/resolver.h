/**
 * @file resolver.h
 * @brief Manages communication with the upstream DNS resolver, including hostname/IP resolution and query forwarding.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#pragma once

#include <string>
#include <cstdint>
#include <netinet/in.h>

sockaddr_storage resolve_host(const std::string &host_name, uint16_t port);
