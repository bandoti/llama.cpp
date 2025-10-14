// Implementation of display management

#include "display.h"
#include "log.h"
#include "console.h"

ConversationDisplay::ConversationDisplay(DisplayRenderer * renderer)
    : current_message_type(MESSAGE_TYPE_USER), in_message(false),
      reasoning_active(false), renderer(renderer) {
}

void ConversationDisplay::begin_message(MessageType type) {
    current_message_type = type;
    in_message = true;
    reasoning_active = false;

    renderer->render_begin(type);
}

void ConversationDisplay::display_diff(const common_chat_msg_diff & diff) {
    // Handle reasoning content
    if (!diff.reasoning_content_delta.empty()) {
        if (!reasoning_active) {
            reasoning_active = true;
            // Begin reasoning message
            begin_message(MESSAGE_TYPE_ASSISTANT_REASONING);
        }
        renderer->render_content(MESSAGE_TYPE_ASSISTANT_REASONING, diff.reasoning_content_delta);
    }

    // Handle regular content
    if (!diff.content_delta.empty()) {
        // If we were displaying reasoning and now have content, end reasoning and start assistant
        if (reasoning_active) {
            reasoning_active = false;
            end_message();  // End reasoning message
            begin_message(MESSAGE_TYPE_ASSISTANT);
        } else if (!in_message) {
            // Auto-start assistant message if not explicitly started
            begin_message(MESSAGE_TYPE_ASSISTANT);
        }
        renderer->render_content(MESSAGE_TYPE_ASSISTANT, diff.content_delta);
    }
}

void ConversationDisplay::end_message() {
    renderer->render_end(current_message_type);
    in_message = false;
    reasoning_active = false;
}

void ConversationDisplay::display_token(const std::string & token_str) {
    renderer->render_bare(token_str);
}

void ConversationDisplay::reset() {
    in_message = false;
    reasoning_active = false;
}

MessageType ConversationDisplay::get_current_message_type() const {
    return current_message_type;
}

bool ConversationDisplay::is_in_message() const {
    return in_message;
}
