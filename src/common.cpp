/**
 * @file tools.cpp
 * @brief Utility functions
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

/// @brief stricter version of stoi, catches letters in string, checks range <0,n>
/// @param str      Input string to parse.
/// @param max_val  Maximum allowed value.
/// @param varname  Name of the variable for error reporting.
/// @return Parsed integer in the valid range.
/// @throws Exits program on failure.
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


/// @brief Appends a single byte to the buffer.
/// @param buf      Destination byte buffer.
/// @param value    Byte to append.
void append_u8(std::vector<uint8_t>& buf, uint8_t value) 
{
    buf.push_back(value);
}

/// @brief Appends 16-bit value to the buffer in network byte order.
/// @param buf      Destination byte buffer.
/// @param value    16-bit value to append .
void append_u16(std::vector<uint8_t>& buf, uint16_t value) 
{
    uint16_t net = htons(value);
    // split into byte array 
    auto ptr = reinterpret_cast<uint8_t*>(&net);
    buf.push_back(ptr[0]);
    buf.push_back(ptr[1]);
}

/// @brief Appends 32-bit value to the buffer in network byte order.
/// @param buf      Destination byte buffer.
/// @param value    32-bit value to append .
void append_u32(std::vector<uint8_t>& buf, uint32_t value) 
{
    uint32_t net = htonl(value);
    // split into byte array 
    auto ptr = reinterpret_cast<uint8_t*>(&net);
    for(size_t i = 0; i < sizeof(net); i++) {
        buf.push_back(ptr[i]);
    }
}

/// @brief  Append string to byte vector.
/// @param buf      Destination byte buffer.
/// @param domain   Domain name (e.g. "example.com").
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

/// @brief Reads 16-bit value from the buffer.
/// @param buf      Source byte buffer.
/// @param offset   Starting position in the buffer.
/// @return Parsed 16-bit value.
/// @note Caller must ensure bounds are valid.
uint16_t read_u16(const std::vector<uint8_t>& buf, size_t offset) 
{
    return ((static_cast<uint16_t>(buf[offset]) << 8) | 
            static_cast<uint16_t>(buf[offset+1]));
}

/// @brief Reads 32-bit value from the buffer.
/// @param buf      Source byte buffer.
/// @param offset   Starting position in the buffer.
/// @return Parsed 32-bit value.
/// @note Caller must ensure bounds are valid.
uint32_t read_u32(const std::vector<uint8_t>& buf, size_t offset)
{
    uint32_t res = 0;
    for (size_t i = 0; i < sizeof(uint32_t); i++) {
        res = (res << 8) | static_cast<uint32_t>(buf[offset+i]);
    }
    return res;
}