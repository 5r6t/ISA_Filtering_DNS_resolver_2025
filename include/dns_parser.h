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
#include <vector>

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
    uint16_t qtype; // 0x01 host addr
    uint16_t qclass; // 0x0001 desired
};

struct DnsMsg {
    DnsHead head;
    DnsQue que;
};

bool read_dns_name(const std::vector<uint8_t>&buf, size_t& offset, std::string& out, 
                    size_t depth=0);
bool read_u16_advance(const std::vector<uint8_t>&buf, size_t& offset, uint16_t& out);
bool parse_dns(const std::vector<uint8_t> &msg, DnsMsg &out);
bool is_A_query(uint16_t qtype);
bool is_error(uint16_t flags);
