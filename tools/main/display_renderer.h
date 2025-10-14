// Display renderer interface for llama-cli
// Defines the bridge interface for different display implementations

#pragma once

#include <string>

// Message types for display categorization
enum MessageType {
    MESSAGE_TYPE_SYSTEM,
    MESSAGE_TYPE_PROMPT,
    MESSAGE_TYPE_USER,
    MESSAGE_TYPE_ASSISTANT,
    MESSAGE_TYPE_ASSISTANT_REASONING,
    MESSAGE_TYPE_TOOL_CALL,
};

// Bridge interface: defines how content should be rendered
// Implementations of this interface handle the actual output (console, file, etc.)
class DisplayRenderer {
public:
    virtual ~DisplayRenderer() = default;

    // Called when a message begins
    virtual void render_begin(MessageType type) = 0;

    // Called when a message ends
    virtual void render_end(MessageType type) = 0;

    // Called to render content (type indicates if it's reasoning, assistant, etc.)
    virtual void render_content(MessageType type, const std::string & content) = 0;

    // Called to render bare tokens (non-parsed mode)
    virtual void render_bare(const std::string & token) = 0;
};
