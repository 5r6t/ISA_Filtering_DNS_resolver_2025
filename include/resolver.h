/**
 * @file resolver.h
 * @brief Hostname/IP resolution
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#pragma once

#include <string>
#include <netinet/in.h>

sockaddr_storage resolve_host(const std::string &host_name, uint16_t port = 53);