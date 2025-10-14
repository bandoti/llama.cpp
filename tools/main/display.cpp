// Implementation of display management

#include "display.h"
#include "log.h"

ConversationDisplay::ConversationDisplay()
    : current_message_type(MESSAGE_TYPE_USER), in_message(false),
      reasoning_char_count(0), reasoning_active(false) {
}

void ConversationDisplay::begin_message(MessageType type) {
    current_message_type = type;
    in_message = true;
    reasoning_char_count = 0;
    reasoning_active = false;

    // Set console colors based on message type
    switch (type) {
        case MESSAGE_TYPE_SYSTEM:
            console::set_display(console::prompt);
            break;
        case MESSAGE_TYPE_USER:
            console::set_display(console::user_input);
            break;
        case MESSAGE_TYPE_ASSISTANT:
            console::set_display(console::reset);
            break;
        case MESSAGE_TYPE_REASONING:
            // Reasoning is handled inline during assistant messages
            break;
        case MESSAGE_TYPE_TOOL_CALL:
            // Tool calls could have special formatting in the future
            break;
    }
}

void ConversationDisplay::display_diff(const common_chat_msg_diff & diff) {
    if (!in_message && current_message_type != MESSAGE_TYPE_ASSISTANT) {
        // Auto-start assistant message if not explicitly started
        begin_message(MESSAGE_TYPE_ASSISTANT);
    }

    // Handle reasoning content
    if (!diff.reasoning_content_delta.empty()) {
        if (!reasoning_active) {
            reasoning_active = true;
            // Could add visual indicator for reasoning start here
        }
        display_reasoning_content(diff.reasoning_content_delta);
    }

    // Handle regular content
    if (!diff.content_delta.empty()) {
        // If we were displaying reasoning and now have content, reasoning phase is done
        if (reasoning_active) {
            reasoning_active = false;
            display_reasoning_summary();
        }
        display_regular_content(diff.content_delta);
    }
}

void ConversationDisplay::end_message() {
    // If reasoning was active and message is ending, show summary
    if (reasoning_active) {
        reasoning_active = false;
        display_reasoning_summary();
    }

    in_message = false;
    console::set_display(console::reset);
}

void ConversationDisplay::display_token(const std::string & token_str) {
    LOG("%s", token_str.c_str());
}

void ConversationDisplay::reset() {
    in_message = false;
    reasoning_char_count = 0;
    reasoning_active = false;
    console::set_display(console::reset);
}

void ConversationDisplay::display_reasoning_content(const std::string & content) {
    reasoning_char_count += content.length();
    // Display reasoning content (could be styled differently in the future)
    LOG("%s", content.c_str());
}

void ConversationDisplay::display_regular_content(const std::string & content) {
    LOG("%s", content.c_str());
}

void ConversationDisplay::display_reasoning_summary() {
    if (reasoning_char_count > 0) {
        LOG("\n[Reasoning: %zu characters]\n", reasoning_char_count);
        reasoning_char_count = 0;
    }
}
