/**
 * @file dns_parser.cpp
 * @brief DNS parsing and error messages building.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#include "dns_parser.h"
#include "common.h"

#include <arpa/inet.h> 

constexpr size_t DEPTH = 10; // Max recursion depth for QNAME parsing
using enum DNSError;

/// @brief 
/// @param client_flags 
/// @param error 
/// @return 
uint16_t make_err_flags(uint16_t client_flags, DNSError error)
{
    uint16_t f = 0; 
    f |= (1 << 15);
    if (client_flags & (1 << 8))
        f |= (1 << 8);
    f |= (static_cast<uint16_t>(error) & 0xF);
    return f;
}

/// @brief 
/// @param err_type 
/// @param  
/// @return 
std::vector<uint8_t> build_error(const DnsMsg &orig, DNSError error) {
    std::vector<uint8_t> out;
    out.reserve(512);
    
    // Header
    append_u16(out, orig.head.id);
    append_u16(out, make_err_flags(orig.head.flags, error));
    /// Counts: Q=1, A=0, NS=0, AR=0
    append_u16(out, 1);
    append_u16(out, 0);
    append_u16(out, 0);
    append_u16(out, 0);

    // Question
    append_dns_name(out, orig.que.qname);
    append_u16(out, orig.que.qtype);
    append_u16(out, orig.que.qclass);

    return out;
}

/// @brief Parse DNS-name into string
/// @param buf      Input buffer with dns message
/// @param offset   Offset within the buffer, updates after successful read.
/// @param out      returns name in string format
/// @param depth    infinite recursion prevention. ignore when calling
/// @return True on success, false if parsing fails or bounds are exceeded.
bool read_dns_name(const std::vector<uint8_t>&buf, size_t& offset, std::string& out, size_t depth)
{
    size_t my_offset = offset; 
    size_t buf_len = buf.size();

    if (my_offset >= buf_len) return false;

    while(my_offset < buf_len) 
    {   
        // length or pointer byte
        const uint8_t len = buf[my_offset]; 

        // Pointer (top 2 bits == 11) 
        if ((len & 0xC0) == 0xC0)
        {
            if (depth >= DEPTH) return false; // recursion limit reached
            if (my_offset + 1 >= buf_len) return false;
                
            size_t jump_to = ((len & 0x3F) << 8) | buf[my_offset + 1];
            depth++; 

            if ( !read_dns_name(buf, jump_to, out, depth))
                return false;

            my_offset += 2; // consume pointer
            break;
        } 

        // end of name
        if (len == 0) 
        {
            my_offset++; // consume null terminator
            break;
        }

        // bounds check
        if (my_offset+len+1 > buf_len) return false; 

        for (size_t j = 1; j <= len; j++) {
            auto c = static_cast<char>(buf[my_offset+j]); 
            out += c;
        }
        out.push_back('.');

        my_offset += len + 1;
    }

    // remove trailing dot
    if (!out.empty() && out.back() == '.') out.pop_back();

    offset = my_offset; // past null terminator
    printf_debug("parsed DNS name (may be partial): %s", out.c_str());
    return true;
}

/// @brief Reads 16-bit value from buffer and advances offset
/// @param buf      Input byte buffer
/// @param offset   Current offset within the buffer; increment by 2 on success
/// @param out      Output parameter to store 16-bit value
/// @return True on success, false if buffer bounds are exceeded
bool read_u16_advance(const std::vector<uint8_t>&buf, size_t& offset, uint16_t& out) {
    if (offset+1 > buf.size()) return false;
    out = read_u16(buf, offset);
    offset += 2;
    return true;
}

/// @brief Reads 32-bit value from buffer and advances offset
/// @param buf      Input byte buffer
/// @param offset   Current offset within the buffer; increment by 4 on success
/// @param out      Output parameter to store 16-bit value
/// @return True on success, false if buffer bounds are exceeded
bool read_u32_advance(const std::vector<uint8_t>&buf, size_t& offset, uint32_t& out) {
    if (offset+1 > buf.size()) return false;
    out = read_u32(buf, offset);
    offset += 4;
    return true;
}

/// @brief 
/// @param qtype 
/// @return 
bool is_A_query (uint16_t qtype) {
    printf_debug("%s", qtype==1 ? "Query type A" : "Query type NOT A"); 
    return qtype == 1;
}

/// @brief 
/// @param flags 
/// @return 
bool is_error(uint16_t flags) {
    uint16_t mask = 0x000F; // lowest 4 bits
    auto rcode = static_cast<DNSError>(mask & flags);
    switch(rcode) {
        case NO_ERR:         printf_debug("Message ok"); return false;
        case FORMAT_ERR:     printf_debug("Format Error: Unable to interpret the query."); break;
        case SRVR_FAIL_ERR:  printf_debug("Server failure."); break;
        case NAME_ERR:       printf_debug("Name Error: Doesn't exist."); break;
        case NOT_IMPLEM_ERR: printf_debug("Not Implemented: No support for such query."); break;
        case REFUSED_ERR:    printf_debug("Refused for policy reasons."); break;
        default:
            printf_debug("Reserved for future use.");
    }
    return true;
}

/// @brief call with size_t offset = 0;
/// @param msg 
/// @param out 
/// @param offset 
/// @return 
bool parse_dns_q(const std::vector<uint8_t> &msg, DnsMsg &out, size_t &offset) {
    bool ok = true;
    if ( !read_u16_advance(msg, offset, out.head.id))       ok=false; 
    if ( !read_u16_advance(msg, offset, out.head.flags))    ok=false; 
    if ( !read_u16_advance(msg, offset, out.head.q_count))  ok=false; 

    offset =  12;
    if ( !read_dns_name   (msg, offset, out.que.qname))     ok=false; 
    if ( !read_u16_advance(msg, offset, out.que.qtype))     ok=false; 
    if ( !read_u16_advance(msg, offset, out.que.qclass))    ok=false; 

    if (ok) {
        printf_debug("ID: %d", out.head.id);
        printf_debug("Flags: %d", out.head.flags);
        printf_debug("QDCOUNT: %d", out.head.q_count);
        printf_debug("QNAME: %s", out.que.qname.c_str());
        printf_debug("QTYPE: %d", out.que.qtype);
        printf_debug("QCLASS:%d", out.que.qclass);
    }
    else
        printf_debug("Couldn't parse DNS question part: bad format");

    return ok;
}


/// @brief call after parse_dns_q and use it's output offset
/// @param msg 
/// @param out 
/// @param offset 
/// @return 
bool parse_dns_a(const std::vector<uint8_t> &msg, DnsMsg &out, size_t &offset) {
    bool ok = true;

    if ( !read_dns_name   (msg, offset, out.answ.aname))      ok=false; 
    if ( !read_u16_advance(msg, offset, out.answ.atype))      ok=false; 
    if ( !read_u16_advance(msg, offset, out.answ.aclass))     ok=false; 
    if ( !read_u32_advance(msg, offset, out.answ.ttl))        ok=false; 
    if ( !read_u16_advance(msg, offset, out.answ.rdlength))   ok=false; 
    if ( !read_u32_advance(msg, offset, out.answ.rdata))      ok=false; 

    if (ok) {
        printf_debug("ANAME: %s", out.answ.aname.c_str());
        printf_debug("Type: %d",  out.answ.atype);
        printf_debug("Class: %d", out.answ.aclass); // 1 is internet addr
        printf_debug("Time to live: %d", out.answ.ttl);    // time to live -- to be cached
        printf_debug("Length of data field: %d", out.answ.rdlength); // expected 4 (octets)
        
        char ip[INET_ADDRSTRLEN];
        uint32_t ip_net = htonl(out.answ.rdata);
        inet_ntop(AF_INET, &ip_net, ip, sizeof(ip));

        printf_debug("Resolved address:%s", ip); // IP addr
    }
    else
        printf_debug("Couldn't parse DNS Answer part: bad format");
    
    return ok;
}

/// @brief 
/// @param msg 
/// @param out 
/// @return 
bool parse_id(const std::vector<uint8_t> &msg, uint16_t &out) {
    if (msg.size() < 2) return false;
    out = read_u16(msg, 0);
    return true;
}