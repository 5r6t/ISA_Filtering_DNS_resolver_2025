/**
 * @file signal.cpp
 * @brief Handles signal setup
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

/// @brief Handles SIGINT and requests shutdown.
/// @param Unused signal number.
void handle_sigint(int /*unused*/) {
    stop_request = true;
    printf_debug("SIGINT detected...\n");
}

/// @brief Installs signal handlers for runtime control.
void setup_signal_handlers() {
    std::signal(SIGINT, handle_sigint);
}