// Display management for llama-cli
// Separates display concerns from processing logic

#pragma once

#include "chat.h"
#include "console.h"
#include <string>

// Message types for display categorization
enum MessageType {
    MESSAGE_TYPE_SYSTEM,
    MESSAGE_TYPE_USER,
    MESSAGE_TYPE_ASSISTANT,
    MESSAGE_TYPE_REASONING,
    MESSAGE_TYPE_TOOL_CALL,
};

// Display lifecycle events
enum DisplayEvent {
    DISPLAY_EVENT_BEGIN,    // Message starts
    DISPLAY_EVENT_CONTENT,  // Streaming content
    DISPLAY_EVENT_END,      // Message ends
};

// Manages display of conversation messages with proper formatting and lifecycle
class ConversationDisplay {
private:
    MessageType current_message_type;
    bool in_message;
    size_t reasoning_char_count;
    bool reasoning_active;

public:
    ConversationDisplay();

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

private:
    // Internal helper to display reasoning content
    void display_reasoning_content(const std::string & content);

    // Internal helper to display regular content
    void display_regular_content(const std::string & content);

    // Internal helper to display reasoning summary
    void display_reasoning_summary();
};
