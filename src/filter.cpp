/**
 * @file filter.cpp
 * @brief Implements domain filtering logic based on the blocklist file and its subdomains.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#include "errors.h"
#include "common.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <ranges>

constexpr int ASCII_MAX = 127;
// lowercase string conversion adapted from:
// https://stackoverflow.com/a/313990
void normalize (std::string &line) {
    std::ranges::transform(line, line.begin(),
        [](unsigned char c){ return std::tolower(c); });
    // careful about trimming empty lines -- is_skippable() called before
    line.erase(0, line.find_first_not_of(" \t\r\n")); // left
    line.erase(line.find_last_not_of(" \t\r\n") + 1); // right
}

bool is_valid_domain (std::string line) {
    if (line.empty()) return false;
    if (line.front() == '.' || line.front() == '-' ||
        line.back() == '.' || line.back() == '-')
        return false;

    if (line.find("..") != std::string::npos) return false;

    bool has_dot = false;
    for (unsigned char c : line) {
        if (c > ASCII_MAX) return false;
        if (c == '.') has_dot = true;
        else if (!std::isalnum(c) && c != '-') return false;
    }
    return has_dot;
}

bool is_skippable(std::string line) {
    return line.empty() || line[0] == '#';
}

std::unordered_set<std::string> filter_load(const std::string& filter_file) 
{
    std::ifstream file(filter_file);

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file\n";
        exit(ERR_FILE);
    }

    std::unordered_set<std::string> blocked;
    std::string line;

    while (std::getline(file, line)) {
        if (is_skippable(line)) 
            continue;

        normalize(line);

        if (!is_valid_domain(line)) {
            std::cerr << "Invalid address found, skipping: " << line << "\n";       
            continue;
        }
        blocked.insert(line);
    }
    file.close();
    printf_debug("Filter list updated");
    return blocked;
}

bool is_blocked(const std::unordered_set<std::string>&blocklist, std::string &domain) {
    if (!is_valid_domain(domain)) {
        printf_debug("Provided domain was invalid");
        return false;
    }
    // get rid of trailing dot 
    if (domain.back() == '.') domain.pop_back();
    // normalize to compare agaisnt the list
    normalize(domain);
    // domain matches the blocklisted domain exactly
    if (blocklist.contains(domain)) {
        printf_debug(" %s is blocked.", domain.c_str());
        return true;
    }
    // subdomain search
    size_t pos = domain.find('.');
    while (pos != std::string::npos) {
    // found match/suffix
        if (blocklist.contains(domain.substr(pos + 1))) {
            printf_debug(" %s is blocked.", domain.c_str());
            return true;
        }
        pos = domain.find('.', pos +1);
    }
    printf_debug(" %s is allowed.", domain.c_str());
    return false; // match not found
}