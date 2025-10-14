// Implementation of conversation management and assistant response parsing

#include "conversation.h"
#include "log.h"

// ConversationManager implementation

ConversationManager::ConversationManager(common_chat_templates * chat_templates, common_params * p)
    : templates(chat_templates), detected_format(COMMON_CHAT_FORMAT_CONTENT_ONLY), params(p) {
}

void ConversationManager::add_system_message(const std::string & content) {
    common_chat_msg msg;
    msg.role = "system";
    msg.content = content;
    messages.push_back(msg);
}

void ConversationManager::add_user_message(const std::string & content) {
    common_chat_msg msg;
    msg.role = "user";
    msg.content = content;
    messages.push_back(msg);
}

void ConversationManager::add_message(const common_chat_msg & msg) {
    messages.push_back(msg);
}

std::vector<llama_token> ConversationManager::get_incremental_tokens(llama_context * ctx) {
    // Apply chat template to get full formatted prompt
    common_chat_templates_inputs inputs;
    inputs.use_jinja = params->use_jinja;
    inputs.messages = messages;
    inputs.add_generation_prompt = !messages.empty() && messages.back().role == "user";
    inputs.reasoning_format = params->reasoning_format;
    bool enable_thinking = params->use_jinja && params->reasoning_budget != 0 &&
                         common_chat_templates_support_enable_thinking(templates);
    inputs.enable_thinking = enable_thinking;

    auto chat_params = common_chat_templates_apply(templates, inputs);
    detected_format = chat_params.format;

    LOG_DBG("Applied chat template, format: %s\n", common_chat_format_name(detected_format));

    // Tokenize the full prompt
    std::vector<llama_token> full_tokens = common_tokenize(ctx, chat_params.prompt, false, true);

    // Find the common prefix with the last tokenized prompt
    size_t common_prefix_len = 0;
    while (common_prefix_len < last_tokenized_prompt.size() &&
           common_prefix_len < full_tokens.size() &&
           last_tokenized_prompt[common_prefix_len] == full_tokens[common_prefix_len]) {
        common_prefix_len++;
    }

    // Extract only the new tokens (the incremental part)
    std::vector<llama_token> incremental_tokens(
        full_tokens.begin() + common_prefix_len,
        full_tokens.end()
    );

    LOG_DBG("Full tokens: %zu, Common prefix: %zu, Incremental: %zu\n",
            full_tokens.size(), common_prefix_len, incremental_tokens.size());

    // Update our cache for next time
    last_tokenized_prompt = full_tokens;

    return incremental_tokens;
}

common_chat_format ConversationManager::get_format() const {
    return detected_format;
}

const std::vector<common_chat_msg> & ConversationManager::get_messages() const {
    return messages;
}

// AssistantResponseParser implementation

AssistantResponseParser::AssistantResponseParser(common_reasoning_format reasoning_fmt) {
    syntax.reasoning_format = reasoning_fmt;
    syntax.parse_tool_calls = false;
}

void AssistantResponseParser::set_format(common_chat_format format) {
    syntax.format = format;
}

void AssistantResponseParser::accumulate_token(const std::string & token_str) {
    accumulator << token_str;
}

std::vector<common_chat_msg_diff> AssistantResponseParser::get_display_diffs() {
    std::string accumulated = accumulator.str();
    common_chat_msg current_parsed = common_chat_parse(accumulated, /* is_partial= */ true, syntax);

    // Compute what changed since last parse
    auto diffs = common_chat_msg_diff::compute_diffs(previous_parsed, current_parsed);
    previous_parsed = current_parsed;

    return diffs;
}

common_chat_msg AssistantResponseParser::finalize() {
    std::string accumulated = accumulator.str();
    return common_chat_parse(accumulated, /* is_partial= */ false, syntax);
}

void AssistantResponseParser::reset() {
    accumulator.str("");
    accumulator.clear();
    previous_parsed = common_chat_msg();
}

bool AssistantResponseParser::has_content() const {
    return !accumulator.str().empty();
}
