/**
 * @file tools.cpp
 * @brief Contains utility functions for ISA DNS project.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-04
 */
#include "../include/common.h"
#include "../include/errors.h"

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <vector>

#define DEPTH 10 // prevent infinite recursion in DNS parsing

/// @brief stricter version of stoi, catches letters in string, checks range <0,n>
int catch_stoi(const std::string &str, const int max_val, const std::string &varname)
{
    try {
        size_t idx;
        int val = std::stoi(str, &idx);
        if (idx != str.size() || val < 0 || val > max_val)
            throw std::out_of_range("Invalid range");
        return val;
    } catch (...) {
        std::cerr << "Error: " << varname <<" must be in range <0,"
                  << max_val << ">" << "\n";
        exit(ERR_BAD_INPUT);
    }
}

/// @brief Append a byte to the byte vector.
void append_uint8(std::vector<uint8_t>& buf, uint8_t value) 
{
    buf.push_back(value);
}
/// @brief Append 2bytes into byte vector in network byte order
void append_uint16(std::vector<uint8_t>& buf, uint16_t value) 
{
    uint16_t net = htons(value);
    // split into byte array 
    auto ptr = reinterpret_cast<uint8_t*>(&net);
    buf.push_back(ptr[0]);
    buf.push_back(ptr[1]);
}
/// @brief Append 4bytes into byte vector in network byte order
void append_uint32(std::vector<uint8_t>& buf, uint32_t value) 
{
    uint32_t net = htonl(value);
    // split into byte array 
    auto ptr = reinterpret_cast<uint8_t*>(&net);
    for(size_t i = 0; i < sizeof(net); i++) {
        buf.push_back(ptr[i]);
    }
}
/// @brief  Append string to byte vector
void append_dns_name(std::vector<uint8_t>& buf, const std::string& domain) 
{
    std::stringstream ss(domain);
    std::string part;
    while (std::getline(ss, part, '.')) {
        buf.push_back(static_cast<uint8_t>(part.size())); // length byte
        buf.insert(buf.end(), part.begin(), part.end()); // label content
    }
    buf.push_back(0); // end of name
}

/**
 * @brief Parse DNS-name into string
 * @param offset (in/out): input start position; update to end of name
 * @return true on success, write to address offset and address out
 */
bool read_dns_name(const std::vector<uint8_t>&buf, size_t& offset, std::string& out, int depth)
{
    size_t my_offset = offset; 
    size_t buf_len = buf.size();

    if (my_offset >= buf_len) return false;

    while(my_offset < buf_len) 
    {
        if ((buf[my_offset] & 0xC0) == 0xC0) // top 2 bits set
        {
            if (depth >= DEPTH) return false; // not so finite recursion reached
            if (my_offset + 1 < buf_len) {
                size_t jump_to = ((buf[my_offset] & 0x3F) << 8) | buf[my_offset + 1];
                depth++; 
                if (!read_dns_name(buf, jump_to, out, depth))
                    return false;
                my_offset += 2; // consumed 2 pointer bytes
                break; // what now TwT
            } 
        } 
        else if (buf[my_offset] == 0) {
            my_offset++; // return new offset + 1
            break;    // if length byte is zero
        }
        uint8_t size = buf[my_offset];        // size = length byte 
        if (my_offset+size+1 > buf_len)     // ERROR: longer than buf
            return false; 

        for (size_t j = 1; j <= size; j++) {
            char c = static_cast<char>(buf[my_offset+j]); 
            out += c;      // 
        }
        out += '.';        // append dot
        my_offset += size+1;   // new offest is offset+size+1
    }
    if (!out.empty() && out.back() == '.') out.pop_back();

    offset = my_offset;
    printf_debug("parsed DNS name (may be partial): %s", out.c_str());
    return true;
}