// Console-based display renderer implementation

#pragma once

#include "display_renderer.h"

// Concrete implementation: console-based renderer
class ConsoleDisplayRenderer : public DisplayRenderer {
public:
    ConsoleDisplayRenderer();

    void render_begin(MessageType type) override;
    void render_end(MessageType type) override;
    void render_content(MessageType type, const std::string & content) override;
    void render_bare(const std::string & token) override;
};
