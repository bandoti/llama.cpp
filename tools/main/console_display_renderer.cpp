// Implementation of console-based display renderer

#include "console_display_renderer.h"
#include "console.h"
#include "log.h"

ConsoleDisplayRenderer::ConsoleDisplayRenderer() {
}

void ConsoleDisplayRenderer::render_begin(MessageType type) {
    // Set console colors based on message type
    switch (type) {
        case MESSAGE_TYPE_SYSTEM:
            console::set_display(console::system);
            break;
        case MESSAGE_TYPE_PROMPT:
            console::set_display(console::prompt);
            break;
        case MESSAGE_TYPE_USER:
            console::set_display(console::user_input);
            break;
        case MESSAGE_TYPE_ASSISTANT:
            console::set_display(console::reset);
            break;
        case MESSAGE_TYPE_ASSISTANT_REASONING:
            // Reasoning is handled inline during assistant messages
            break;
        case MESSAGE_TYPE_TOOL_CALL:
            // Tool calls could have special formatting in the future
            break;
    }
}

void ConsoleDisplayRenderer::render_end(MessageType type) {
    // Reset console display after message ends
    console::set_display(console::reset);
}

void ConsoleDisplayRenderer::render_content(MessageType type, const std::string & content) {
    // For now, all content is displayed the same way
    // In the future, reasoning content could be styled differently
    LOG("%s", content.c_str());
}

void ConsoleDisplayRenderer::render_bare(const std::string & token) {
    LOG("%s", token.c_str());
}
