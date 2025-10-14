// Conversation management and assistant response parsing for llama-cli

#pragma once

#include "chat.h"
#include "common.h"
#include <vector>
#include <string>
#include <sstream>

// Manages conversation state and produces incremental token streams
// Handles the stateless message accumulation pattern where full templates
// are applied each time, but only new tokens are returned for efficiency
class ConversationManager {
private:
    std::vector<common_chat_msg> messages;
    common_chat_templates * templates;
    std::vector<llama_token> last_tokenized_prompt;
    common_chat_format detected_format;
    common_params * params;

public:
    ConversationManager(common_chat_templates * chat_templates, common_params * p);

    // Add a system message to the conversation
    void add_system_message(const std::string & content);

    // Add a user message to the conversation
    void add_user_message(const std::string & content);

    // Add a complete message to the conversation
    void add_message(const common_chat_msg & msg);

    // Get the incremental tokens needed since last call
    // Returns only NEW tokens by comparing with last_tokenized_prompt
    // This implements the stateless-but-efficient pattern:
    // - Applies full chat template to all messages
    // - Tokenizes the complete formatted prompt
    // - Returns only tokens beyond the last cached position
    std::vector<llama_token> get_incremental_tokens(llama_context * ctx);

    // Get the detected chat format after template application
    common_chat_format get_format() const;

    // Get the current messages
    const std::vector<common_chat_msg> & get_messages() const;
};

// Handles streaming assistant response with reasoning support
// Accumulates tokens, parses incrementally to separate reasoning from content,
// and provides diffs for streaming display
class AssistantResponseParser {
private:
    std::ostringstream accumulator;
    common_chat_msg previous_parsed;
    common_chat_syntax syntax;

public:
    AssistantResponseParser(common_reasoning_format reasoning_fmt);

    // Set the chat format (must be called after format detection)
    void set_format(common_chat_format format);

    // Accumulate a new token from the model
    void accumulate_token(const std::string & token_str);

    // Parse incrementally and return diffs for display
    // Returns differences since last parse (reasoning_content_delta and content_delta)
    std::vector<common_chat_msg_diff> get_display_diffs();

    // Get the complete parsed message (call at EOG)
    // Parses with is_partial=false to get final message
    common_chat_msg finalize();

    // Reset for next response
    void reset();

    // Check if any content has been accumulated
    bool has_content() const;
};
