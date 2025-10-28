/**
 * @file dns_parser.cpp
 * @brief Provides functions for parsing and constructing DNS query and response messages.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#include "dns_parser.h"
#include "common.h"

constexpr size_t DEPTH = 10; // Max recursion depth for QNAME parsing

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
    if (offset+1 >= buf.size()) return false;
    out = read_u16(buf, offset);
    offset += 2;
    return true;
}

bool is_A_query (uint16_t qtype) {
    printf_debug("%s", qtype==1 ? "Query type is A" : "Query type is smthn else"); 
    return qtype == 1;
}

bool parse_dns(const std::vector<uint8_t> &msg, DnsMsg &out) {
    bool ok = true;
    size_t offset =  0;
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
        printf_debug("Flags: %d", out.head.q_count);
        printf_debug("QNAME: %s", out.que.qname.c_str());
        printf_debug("QTYPE: %d", out.que.qtype);
        printf_debug("QCLASS:%d", out.que.qclass);
    }
    else {
        printf_debug("Couldn't parse DNS: bad format");
    }

    return ok;
}