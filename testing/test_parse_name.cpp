/**
 * @file test_parse_name.cpp
 * @brief test file to check correctness of dns name parsing
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-11-16
 */

#include <iostream>
#include <vector>
#include <string>

#include "common.h"
#include "dns_parser.h"  

int main() {

    {
        std::cout << ">>> TEST 1: simple name <<<\n";
        const std::vector<uint8_t> buf = {
            0x03, 'w','w','w',
            0x06, 'g','o','o','g','l','e',
            0x03, 'c','o','m',
            0x00
        };

        size_t offset = 0;
        std::string out;

        if (read_dns_name(buf, offset, out))
            std::cout << "Parsed name: " << out << "\n";
        else
            std::cout << "Failed to parse name!\n";

        std::cout << "\n";
    }

    {
        std::cout << ">>> TEST 2: name with pointer <<<\n";
        const std::vector<uint8_t> buf2 = {
            // offset 0:
            0x03,'w','w','w', 0xC0,0x06,

            // offset 6:
            0x06,'g','o','o','g','l','e',
            0x03,'c','o','m',
            0x00
        };

        size_t offset = 0;
        std::string out;

        if (read_dns_name(buf2, offset, out))
            std::cout << "Parsed name: " << out << "\n";
        else
            std::cout << "Failed to parse name!\n";

        std::cout << "\n";
    }

    {
        std::cout << ">>> TEST 3: multiple pointers <<<\n";
        std::vector<uint8_t> buf = {
            // 0x00:
            0x06,'g','o','o','g','l','e',
            0x03,'c','o','m',0x00,              // google.com

            // 0x0C:
            0x03,'w','w','w',0xC0,0x00,         // www.google.com

            // 0x12:
            0x04,'m','a','i','l',0xC0,0x00,     // mail.google.com

            // 0x18:
            0x03,'a','p','i',0xC0,0x00          // api.google.com
        };

        std::string out;
        size_t offset;

        offset = 12;
        out.clear();
        if (read_dns_name(buf, offset, out))
            std::cout << "Parsed name1: " << out << "\n";
        else
            std::cout << "Failed to parse name1!\n";

        offset = 18;
        out.clear();
        if (read_dns_name(buf, offset, out))
            std::cout << "Parsed name2: " << out << "\n";
        else
            std::cout << "Failed to parse name2!\n";

        offset = 25;
        out.clear();
        if (read_dns_name(buf, offset, out))
            std::cout << "Parsed name3: " << out << "\n";
        else
            std::cout << "Failed to parse name3!\n";

        std::cout << "\n";
    }
    
    return 0;
}
