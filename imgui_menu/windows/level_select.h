#pragma once
#include <cfloat>
#include <cstdio>
#include "../components/window.h"

extern "C" {
    extern int16_t gCurrLevelNum;
    int32_t level_load(int16_t level_num);
    const char *level_get_string(void);
}

#define STUB_LEVEL(...)
#define DEFINE_LEVEL(name, e, ...) { (int16_t)(e), name },
namespace { struct LvlEntry { int16_t num; const char *name; }; }
static const LvlEntry k_lv_entries[] = {
    #include "levels/level_defines.h"
};
#undef STUB_LEVEL
#undef DEFINE_LEVEL
static constexpr int k_lv_count = (int)(sizeof(k_lv_entries) / sizeof(k_lv_entries[0]));

class LevelSelect : public Window {
public:
    LevelSelect() : Window("##levelselect", ImVec2(360.f, 0.f),
        ImGuiWindowFlags_NoDecoration       |
        ImGuiWindowFlags_NoSavedSettings    |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav              |
        ImGuiWindowFlags_NoMove             |
        ImGuiWindowFlags_NoResize) {}

    void open() override {
        m_selected = -1;
        for (int i = 0; i < k_lv_count; i++) {
            if (k_lv_entries[i].num == gCurrLevelNum) { m_selected = i; break; }
        }
        if (m_selected < 0 && k_lv_count > 0) m_selected = 0;
        Window::open();
    }

protected:
    void position_window() override {
        ImGuiIO &io = ImGui::GetIO();
        ImGui::SetNextWindowPos(
            ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(360.f, 0.f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.92f);
    }

    void draw_content() override {
        ImGui::TextUnformatted("Level Select");
        ImGui::TextDisabled("%s", level_get_string());
        ImGui::Spacing();

        if (ImGui::BeginListBox("Maps", ImVec2(-FLT_MIN, 220.f))) {
            for (int i = 0; i < k_lv_count; i++) {
                bool sel = (m_selected == i);
                if (ImGui::Selectable(k_lv_entries[i].name, sel)) m_selected = i;
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }

        if (m_selected >= 0 && m_selected < k_lv_count) {
            const LvlEntry &e = k_lv_entries[m_selected];
            if (e.num != gCurrLevelNum) {
                if (ImGui::Button("Load", ImVec2(-FLT_MIN, 0.f))) {
                    std::fprintf(stderr, "[level-select] %d -> %d (%s)\n",
                                 (int)gCurrLevelNum, (int)e.num, e.name);
                    level_load(e.num);
                    close();
                }
            } else {
                ImGui::BeginDisabled();
                ImGui::Button("Already In This Map", ImVec2(-FLT_MIN, 0.f));
                ImGui::EndDisabled();
            }
        }
        ImGui::Spacing();
        if (ImGui::Button("Close")) close();
    }

private:
    int m_selected = -1;
};
