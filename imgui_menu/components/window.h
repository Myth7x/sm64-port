#pragma once
#include "base/component.h"

class Window : public Component {
public:
    explicit Window(const char *title, ImVec2 initial_size = ImVec2(0.f, 0.f),
                    ImGuiWindowFlags extra_flags = ImGuiWindowFlags_NoSavedSettings)
        : m_title(title), m_initial_size(initial_size), m_flags(extra_flags) {}

    void init() override {}
    void shutdown() override {}

    void compose_frame() override {
        if (!m_visible) return;
        position_window();
        ImGui::Begin(m_title, &m_visible, m_flags);
        draw_content();
        ImGui::End();
    }

    virtual void open()   { m_visible = true; }
    void close()  { m_visible = false; }
    void toggle() { m_visible = !m_visible; }
    bool is_open() const { return m_visible; }

protected:
    virtual void position_window() {
        if (m_initial_size.x > 0.f || m_initial_size.y > 0.f)
            ImGui::SetNextWindowSize(m_initial_size, ImGuiCond_FirstUseEver);
    }
    virtual void draw_content() = 0;

    bool             m_visible      = false;
    const char      *m_title;
    ImVec2           m_initial_size;
    ImGuiWindowFlags m_flags;
};