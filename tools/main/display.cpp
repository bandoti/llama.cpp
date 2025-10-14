// Implementation of display management

#include "display.h"
#include "log.h"
#include "console.h"

ConversationDisplay::ConversationDisplay(DisplayRenderer * renderer)
    : current_message_type(MESSAGE_TYPE_USER), in_message(false),
      reasoning_char_count(0), reasoning_active(false), renderer(renderer) {
}

void ConversationDisplay::begin_message(MessageType type) {
    current_message_type = type;
    in_message = true;
    reasoning_char_count = 0;
    reasoning_active = false;

    renderer->render_begin(type);
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

    renderer->render_end(current_message_type);
    in_message = false;
}

void ConversationDisplay::display_token(const std::string & token_str) {
    renderer->render_bare(token_str);
}

void ConversationDisplay::reset() {
    in_message = false;
    reasoning_char_count = 0;
    reasoning_active = false;
}

MessageType ConversationDisplay::get_current_message_type() const {
    return current_message_type;
}

bool ConversationDisplay::is_in_message() const {
    return in_message;
}

void ConversationDisplay::display_reasoning_content(const std::string & content) {
    reasoning_char_count += content.length();
    renderer->render_content(MESSAGE_TYPE_ASSISTANT_REASONING, content);
}

void ConversationDisplay::display_regular_content(const std::string & content) {
    renderer->render_content(MESSAGE_TYPE_ASSISTANT, content);
}

void ConversationDisplay::display_reasoning_summary() {
    if (reasoning_char_count > 0) {
        // Pass summary as content with reasoning type and char count in the message
        renderer->render_content(MESSAGE_TYPE_ASSISTANT_REASONING, "\n[Reasoning: " + std::to_string(reasoning_char_count) + " characters]\n");
        reasoning_char_count = 0;
    }
}
