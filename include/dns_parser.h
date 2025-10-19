/**
 * @file dns_parser.h
 * @brief Provides functions for parsing and constructing DNS query and response messages.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#pragma once

#include <cstdint>
#include <string>

struct DnsMsg {
    DnsHead header;
    DnsQue question;
};

// header has 12 bytes
struct DnsHead
{
    // 2B each
    uint16_t id;
    uint16_t flags;
    uint16_t q_count;
    // rest ommited
}; 

struct DnsQue
{
    std::string qname;
    uint16_t qtype; // 0x0001 host addr
    uint16_t qclass; // 0x00001 desired
};