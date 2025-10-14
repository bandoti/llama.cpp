// Console manager for llama-cli
// Provides a higher-level abstraction over the common console functions
// and integrates with the display renderer system

#pragma once

#include "console.h"
#include "display_renderer.h"
#include <string>

// Forward declarations for types we need to store
struct llama_context;
struct llama_model;
struct common_sampler;
struct common_params;

class ConsoleManager {
public:
    // Get singleton instance
    static ConsoleManager & instance();

    // Initialize console
    void init(bool use_simple_io, bool use_color);

    // Cleanup console
    void cleanup();

    // Read a line of input from the user
    bool readline(std::string & line, bool multiline_input);

    // Read user input into a buffer (handles multiline input loop)
    void read_user_input(std::string & buffer, bool multiline_input);

    // Set display mode for error messages
    void set_error_display();

    // Reset to clean state (useful before starting message display)
    void reset_display();

    // Set context for signal handler cleanup
    void set_cleanup_context(llama_context ** ctx, common_sampler ** smpl, common_params * params,
                             bool * is_interacting_ptr, bool * need_insert_eot_ptr);

    // Setup signal handlers
    void setup_signal_handlers();

    // Static signal handler callback
    static void signal_handler(int signo);

private:
    ConsoleManager();
    ~ConsoleManager() = default;

    // Prevent copying
    ConsoleManager(const ConsoleManager &) = delete;
    ConsoleManager & operator=(const ConsoleManager &) = delete;

    // Context needed for cleanup
    llama_context ** ctx = nullptr;
    common_sampler ** smpl = nullptr;
    common_params * params = nullptr;
    bool * is_interacting = nullptr;
    bool * need_insert_eot = nullptr;
};
