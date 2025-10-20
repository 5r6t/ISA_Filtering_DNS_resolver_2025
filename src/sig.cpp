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
#include "common.h"

#include <csignal>
#include <atomic>
#include <iostream>
#include <unistd.h>
#include <vector>

std::atomic<bool>stop_request = false;

void handle_sigint(int) {
    stop_request = true;
    printf_debug("SIGINT detected...\n");
}

void setup_signal_handlers() {
    std::signal(SIGINT, handle_sigint);
}