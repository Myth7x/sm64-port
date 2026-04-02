#if defined(ENABLE_DX11) || defined(ENABLE_DX12) || defined(TARGET_LINUX)

#include <cmath>
#include "imgui.h"
#include "level_select_menu.h"

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
extern "C" {
#include "sm64.h"
#include "types.h"
#include "game/fps_mode.h"
#include "game/fps_camera.h"
#include "game/level_update.h"
#include "engine/surface_load.h"
#include "game/object_list_processor.h"
#include "pc/mouse.h"
#include "game/mario.h"
#include "game/game_init.h"
}

#include "pc/load_progress.h"

static bool s_levelSelectOpen = false;

void level_select_menu_init(void) {
}

void level_select_menu_shutdown(void) {
}

void level_select_menu_new_frame(void) {
}

void level_select_menu_open(void) {
    s_levelSelectOpen = true;
}

void level_select_menu_close(void) {
    s_levelSelectOpen = false;
}

static void draw_level_select_menu_modal(void) {
    if (!s_levelSelectOpen) return;

    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(360.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.92f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration       |
        ImGuiWindowFlags_NoSavedSettings    |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav              |
        ImGuiWindowFlags_NoMove             |
        ImGuiWindowFlags_NoResize;

    bool keepOpen = s_levelSelectOpen;
    ImGui::Begin("##levelselectmenu", &keepOpen, flags);
    ImGui::TextUnformatted("Level Select Menu");
    ImGui::TextDisabled("%s", level_get_string());
    ImGui::Spacing();
    if (ImGui::Button("Close")) {
        keepOpen = false;
    }
    ImGui::End();
    s_levelSelectOpen = keepOpen;
}

void level_select_menu_compose_frame(void) {
    draw_level_select_menu_modal();
}

void level_select_menu_render(void) {
}

#else

void level_select_menu_init(void) {}
void level_select_menu_shutdown(void) {}
void level_select_menu_new_frame(void) {}
void level_select_menu_open(void) {}
void level_select_menu_close(void) {}
void level_select_menu_compose_frame(void) {}
void level_select_menu_render(void) {}

#endif
