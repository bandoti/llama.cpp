// Implementation of console manager

#include "console_manager.h"
#include "common.h"
#include "llama.h"
#include "log.h"
#include "sampling.h"

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <signal.h>
#include <unistd.h>
#elif defined (_WIN32)
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <signal.h>
#endif

ConsoleManager::ConsoleManager() {
}

ConsoleManager & ConsoleManager::instance() {
    static ConsoleManager instance;
    return instance;
}

void ConsoleManager::init(bool use_simple_io, bool use_color) {
    console::init(use_simple_io, use_color);
}

void ConsoleManager::cleanup() {
    console::cleanup();
}

bool ConsoleManager::readline(std::string & line, bool multiline_input) {
    return console::readline(line, multiline_input);
}

void ConsoleManager::read_user_input(std::string & buffer, bool multiline_input) {
    buffer.clear();
    std::string line;
    bool another_line = true;
    do {
        another_line = console::readline(line, multiline_input);
        buffer += line;
    } while (another_line);
}

void ConsoleManager::set_error_display() {
    console::set_display(console::error);
}

void ConsoleManager::reset_display() {
    console::set_display(console::reset);
}

void ConsoleManager::set_cleanup_context(llama_context ** ctx, common_sampler ** smpl, common_params * params,
                                          bool * is_interacting_ptr, bool * need_insert_eot_ptr) {
    this->ctx = ctx;
    this->smpl = smpl;
    this->params = params;
    this->is_interacting = is_interacting_ptr;
    this->need_insert_eot = need_insert_eot_ptr;
}

void ConsoleManager::setup_signal_handlers() {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
    struct sigaction sigint_action;
    sigint_action.sa_handler = ConsoleManager::signal_handler;
    sigemptyset(&sigint_action.sa_mask);
    sigint_action.sa_flags = 0;
    sigaction(SIGINT, &sigint_action, NULL);
#elif defined (_WIN32)
    auto console_ctrl_handler = +[](DWORD ctrl_type) -> BOOL {
        return (ctrl_type == CTRL_C_EVENT) ? (ConsoleManager::signal_handler(SIGINT), true) : false;
    };
    SetConsoleCtrlHandler(reinterpret_cast<PHANDLER_ROUTINE>(console_ctrl_handler), true);
#endif
}

void ConsoleManager::signal_handler(int signo) {
    if (signo == SIGINT) {
        ConsoleManager & mgr = ConsoleManager::instance();

        if (!*mgr.is_interacting && mgr.params->interactive) {
            *mgr.is_interacting = true;
            *mgr.need_insert_eot = true;
        } else {
            mgr.cleanup();
            LOG("\n");

            if (mgr.ctx && mgr.smpl) {
                common_perf_print(*mgr.ctx, *mgr.smpl);
            }

            // make sure all logs are flushed
            LOG("Interrupted by user\n");
            common_log_pause(common_log_main());

            _exit(130);
        }
    }
}
