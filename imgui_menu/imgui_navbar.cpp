#if defined(ENABLE_DX11) || defined(ENABLE_DX12) || defined(TARGET_LINUX)

#include "imgui.h"
#include "imgui_navbar.h"
#include "level_config.h"
#include "level_select_menu.h"
#include "physics_config.h"

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
extern "C" {
#include "pc/mouse.h"
}

static bool s_infoWindowVisible = true;

void imgui_navbar_init(void) {
}

void imgui_navbar_shutdown(void) {
}

static void draw_navbar(void) {
    ImGuiIO &io = ImGui::GetIO();
    bool window_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
    bool any_item_interaction = ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive();
    bool should_hide_navbar = (io.WantCaptureMouse && window_hovered) || (window_hovered && !any_item_interaction);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration       |
        ImGuiWindowFlags_AlwaysAutoResize   |
        ImGuiWindowFlags_NoSavedSettings    |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav              |
        ImGuiWindowFlags_NoMove             |
        ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.85f);

    ImGui::Begin("##navbar", nullptr, flags);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));

    ImGui::TextUnformatted("SM64 Port");
    ImGui::SameLine(150.0f);

    if (ImGui::Button("Level Select", ImVec2(120.0f, 0.0f))) {
        level_select_menu_open();
    }

    ImGui::SameLine();
    if (ImGui::Button(s_infoWindowVisible ? "Hide Info" : "Show Info", ImVec2(100.0f, 0.0f))) {
        s_infoWindowVisible = !s_infoWindowVisible;
    }

    ImGui::SameLine();
    if (ImGui::Button("Physics", ImVec2(80.0f, 0.0f))) {
        physics_config_toggle();
    }

    ImGui::SameLine();
    if (ImGui::Button("Level Config", ImVec2(110.0f, 0.0f))) {
        level_config_toggle();
    }

    ImGui::PopStyleVar();
    ImGui::End();
}

void imgui_navbar_compose_frame(void) {
    draw_navbar();
    physics_config_compose_frame();
    level_config_compose_frame();
}

bool imgui_navbar_get_info_visible(void) {
    return s_infoWindowVisible;
}

#else

void imgui_navbar_init(void) {}
void imgui_navbar_shutdown(void) {}
void imgui_navbar_compose_frame(void) {}

#endif
