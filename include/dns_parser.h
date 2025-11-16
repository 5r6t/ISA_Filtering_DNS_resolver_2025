/**
 * @file dns_parser.h
 * @brief parsing and error messages building.
 * 
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class DNSError {
    NO_ERR, FORMAT_ERR, SRVR_FAIL_ERR, NAME_ERR, NOT_IMPLEM_ERR, REFUSED_ERR  
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
    uint16_t qtype; // 0x01 host addr
    uint16_t qclass; // 0x0001 desired
};

struct DnsAnsw 
{
    std::string aname;
    uint16_t atype; // type of rdata, we want x0001
    uint16_t aclass;
    uint32_t ttl; // number of seconds the res can be cached
    uint16_t rdlength; // length of rdata
    // assuming we are working with A records only
    // then 4 octets = ip_addr
    uint32_t rdata;  
};

struct DnsMsg {
    DnsHead head;
    DnsQue que;
    DnsAnsw answ;
};

std::vector<uint8_t> build_error(const DnsMsg &orig, DNSError error);
bool read_dns_name(const std::vector<uint8_t>&buf, size_t& offset, std::string& out, 
                    size_t depth=0);
bool read_u16_advance(const std::vector<uint8_t>&buf, size_t& offset, uint16_t& out);
bool read_u32_advance(const std::vector<uint8_t>&buf, size_t& offset, uint32_t& out);
bool parse_id(const std::vector<uint8_t> &msg, uint16_t &out);
bool parse_dns_a(const std::vector<uint8_t> &msg, DnsMsg &out, size_t& offset);
bool parse_dns_q(const std::vector<uint8_t> &msg, DnsMsg &out, size_t& offset);
bool is_A_query(uint16_t qtype);
bool is_error(uint16_t flags);