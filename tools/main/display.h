// Display management for llama-cli
// Separates display concerns from processing logic using the Bridge pattern

#pragma once

#include "chat.h"
#include "display_renderer.h"
#include <string>

// Abstraction: manages display logic and delegates rendering to the implementation
class ConversationDisplay {
private:
    MessageType current_message_type;
    bool in_message;
    size_t reasoning_char_count;
    bool reasoning_active;
    DisplayRenderer * renderer;

public:
    explicit ConversationDisplay(DisplayRenderer * renderer);

    // Signal the beginning of a message
    void begin_message(MessageType type);

    // Display content delta from the parser
    void display_diff(const common_chat_msg_diff & diff);

    // Signal the end of a message
    void end_message();

    // Display a complete token (for non-parsed output)
    void display_token(const std::string & token_str);

    // Reset state for new conversation turn
    void reset();

    // Get the current message type
    MessageType get_current_message_type() const;

    // Check if currently in a message
    bool is_in_message() const;

private:
    // Internal helper to display reasoning content
    void display_reasoning_content(const std::string & content);

    // Internal helper to display regular content
    void display_regular_content(const std::string & content);

    // Internal helper to display reasoning summary
    void display_reasoning_summary();
};
