/**
 * @file signal.cpp
 * @brief Handles signal setup and graceful program termination (e.g., SIGINT cleanup).
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */
#include "sig.h"
#include "errors.h"
#include "tools.h"
#include <csignal>
#include <atomic>
#include <iostream>
#include <unistd.h>
#include <vector>

std::atomic<bool>stop_request = false;
static std::vector<void (*)()> cleanup_functions;

void handle_sigint(int) {
    stop_request = true;
    printf_debug("\nSIGINT deteced, attempting graceful shutdown....\n");
}

void setup_signal_handlers() {
    std::signal(SIGINT, handle_sigint);
}

void add_cleanup(void (*fn)()) {
    cleanup_functions.push_back(fn);
}

void cleanup() {
    std::cerr << "\nRunning registered cleanup functions...\n";
    for (auto fn : cleanup_functions)
        fn();  // run each registered cleanup
}