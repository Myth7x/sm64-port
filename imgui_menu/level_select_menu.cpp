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

static unsigned long long s_lastPerfCount  = 0;
static int                s_fps            = 0;
static int                s_loadFramesLeft = 0;

void level_select_menu_init(void) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::StyleColorsDark();
    ImGuiStyle &style    = ImGui::GetStyle();
    style.WindowRounding  = 4.0f;
    style.WindowBorderSize = 0.0f;
}

void level_select_menu_shutdown(void) {
    ImGui::DestroyContext();
}

void level_select_menu_new_frame(void) {
    ImGui::NewFrame();
}

static void draw_info_box(void) {
    if (!gFPSMode) return;
    if (gMarioState == nullptr) return;

    unsigned long long now  = pc_perf_counter();
    unsigned long long freq = pc_perf_freq();
    if (s_lastPerfCount != 0 && freq != 0) {
        unsigned long long diff = now - s_lastPerfCount;
        s_fps = (diff > 0) ? (int)(freq / diff) : 0;
    }
    s_lastPerfCount = now;

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration       |
        ImGuiWindowFlags_AlwaysAutoResize   |
        ImGuiWindowFlags_NoSavedSettings    |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav              |
        ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos(ImVec2(8.0f, 8.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.65f);
    ImGui::Begin("##fps_info", nullptr, flags);

    ImGui::Text("FPS  %d", s_fps);
    ImGui::Text("MAP  %s", level_get_string());
    ImGui::Text("POS  %.0f  %.0f  %.0f",
        (double)gMarioState->pos[0],
        (double)gMarioState->pos[1],
        (double)gMarioState->pos[2]);
    {
        const f32 *vel = gMarioState->vel;
        ImGui::Text("SPD  %d", (int)(sqrtf(vel[0]*vel[0] + vel[2]*vel[2]) * 30.0f));
    }
    ImGui::Separator();
    s32 dyn_surfs = gSurfacesAllocated - gNumStaticSurfaces;
    s32 dyn_nodes = gSurfaceNodesAllocated - gNumStaticSurfaceNodes;
    ImGui::Text("TRI  S:%-5d D:%d", gNumStaticSurfaces, dyn_surfs);
    ImGui::Text("NOD  S:%-5d D:%d", gNumStaticSurfaceNodes, dyn_nodes);
    ImGui::Text("POOL %d",           gSurfacesAllocated);
    ImGui::Text("VTX  %d",           gNumStaticSurfaces * 3);
    ImGui::End();
}

static void draw_level_select_menu_modal(void) {
    if (gParallelLoadActive.load()) s_loadFramesLeft = 90;
    if (s_loadFramesLeft <= 0) return;
    s_loadFramesLeft--;

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

    ImGui::Begin("##levelselectmenu", nullptr, flags);
    ImGui::TextUnformatted("NEED_IMPLEMENTATION");
    ImGui::Spacing();
    ImGui::End();
}

void level_select_menu_compose_frame(void) {
    draw_info_box();
    draw_level_select_menu_modal();
}

void level_select_menu_render(void) {
    ImGui::Render();
}

#else

void level_select_menu_init(void) {}
void level_select_menu_shutdown(void) {}
void level_select_menu_new_frame(void) {}
void level_select_menu_compose_frame(void) {}
void level_select_menu_render(void) {}

#endif
