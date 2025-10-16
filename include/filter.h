/**
 * @file filter.h
 * @brief Implements domain filtering logic based on the blocklist file and its subdomains.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

 #pragma once
 #include <string>
 #include <unordered_set>

void normalize (std::string &line);   // remove after testing
bool is_skippable (std::string line); // remove after testing 
bool is_blocked (std::unordered_set<std::string>& blocklist, std::string& domain);
std::unordered_set<std::string> filter_load(std::string filter_file);
