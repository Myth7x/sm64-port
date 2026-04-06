#if defined(ENABLE_DX11) || defined(ENABLE_DX12) || defined(TARGET_LINUX)

#include <cmath>
#include <cstdio>
#include <cstdint>
#include "imgui.h"
#include "imgui_menu.h"
#include "toast_c.h"

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
extern "C" {
#include "types.h"
#include "sm64.h"
#include "game/fps_camera.h"
#include "game/area.h"
#include "game/level_update.h"
#include "game/main.h"
#include "game/camera.h"
#include "game/game_init.h"
#include "engine/surface_load.h"
#include "pc/gfx/gfx_pc.h"
#include "src/game/source_movement.h"
}

#include "pc/load_progress.h"

#include "components/base/component.h"
#include "components/window.h"
#include "components/toast.h"
#include "windows/navbar.h"
#include "windows/level_select.h"
#include "windows/level_debug.h"
#include "windows/physics_config.h"
#include "windows/shader_config.h"

static Toast         s_toast;
static NavBar        s_navbar;
static LevelSelect   s_level_select;
static LevelDebug    s_level_debug;
static PhysicsConfig s_physics;
static ShaderConfig  s_shader;

static int s_load_frames_left = 0;

static void draw_loading_modal(void) {
    bool level_active   = (gLevelLoadingActive != 0);
    bool parallel_active = gParallelLoadActive.load();
    if (level_active || parallel_active) s_load_frames_left = 90;
    if (s_load_frames_left <= 0) return;
    s_load_frames_left--;

    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.7f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480.f, 0.f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.95f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration       |
        ImGuiWindowFlags_NoSavedSettings    |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav              |
        ImGuiWindowFlags_NoMove             |
        ImGuiWindowFlags_NoResize;

    ImGui::Begin("##loading", nullptr, flags);

    LoadPhase phase      = (LoadPhase)gLoadPhase.load();
    int       total      = gParallelLoadTotal.load();
    int       done       = gParallelLoadDone.load();
    int       threads    = gLoadThreadCount.load();
    int64_t   elapsed_ms = gLoadElapsedMs.load();
    int64_t   eta_ms     = gLoadEtaMs.load();

    ImGui::SetWindowFontScale(1.2f);
    if (level_active && phase <= LOAD_PHASE_PARSE)
        ImGui::TextUnformatted("Loading Level...");
    else if (parallel_active || (level_active && phase >= LOAD_PHASE_COMPUTE))
        ImGui::TextUnformatted("Processing Geometry...");
    else
        ImGui::TextUnformatted("Level Ready!");
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::TextDisabled("Map: %s", level_get_string());
    ImGui::Spacing();

    struct PhaseInfo { const char *label; LoadPhase id; };
    static const PhaseInfo phases[] = {
        { "Parse",   LOAD_PHASE_PARSE   },
        { "Compute", LOAD_PHASE_COMPUTE },
        { "Insert",  LOAD_PHASE_INSERT  },
    };
    ImDrawList *dl    = ImGui::GetWindowDrawList();
    ImVec2      cur   = ImGui::GetCursorScreenPos();
    float       sw    = 480.f / 3.f;
    float       dot_r = 5.f;
    float       cy    = cur.y + dot_r + 2.f;

    for (int pi = 0; pi < 3; pi++) {
        float cx         = cur.x + sw * (pi + 0.5f);
        bool  act        = (phase == phases[pi].id);
        bool  done_phase = (phase > phases[pi].id);
        ImU32 col = done_phase ? IM_COL32(100,220,100,255) :
                    act        ? IM_COL32(255,200, 60,255) :
                                 IM_COL32(120,120,120,255);
        if (pi > 0) {
            float pcx = cur.x + sw * (pi - 0.5f);
            ImU32 lc = (phase > phases[pi-1].id) ? IM_COL32(100,220,100,160) : IM_COL32(100,100,100,100);
            dl->AddLine(ImVec2(pcx + dot_r, cy), ImVec2(cx - dot_r, cy), lc, 1.5f);
        }
        dl->AddCircleFilled(ImVec2(cx, cy), dot_r, col);
        if (act) dl->AddCircle(ImVec2(cx, cy), dot_r + 2.f, IM_COL32(255,255,255,180), 0, 1.5f);
        ImVec2 ts = ImGui::CalcTextSize(phases[pi].label);
        ImU32  tc = done_phase ? IM_COL32(150,255,150,255) :
                    act        ? IM_COL32(255,220,100,255) :
                                 IM_COL32(160,160,160,255);
        dl->AddText(ImVec2(cx - ts.x * 0.5f, cy + dot_r + 4.f), tc, phases[pi].label);
    }
    ImGui::Dummy(ImVec2(0.f, dot_r * 2.f + 18.f));

    float frac = (total > 0) ? (float)done / (float)total
                             : (level_active || parallel_active ? 0.f : 1.f);
    char prog[32];
    snprintf(prog, sizeof(prog), "%.0f%%", frac * 100.f);
    ImGui::ProgressBar(frac, ImVec2(-1.f, 0.f), prog);

    ImGui::Spacing();
    if (total > 0) ImGui::TextDisabled("%d / %d tris", done, total);
    if (threads > 0 || elapsed_ms > 0) {
        ImGui::SameLine(ImGui::GetWindowWidth() * 0.55f);
        if (threads > 0 && (parallel_active || phase == LOAD_PHASE_COMPUTE)) {
            if (eta_ms > 0 && done < total)
                ImGui::TextDisabled("%d threads  ETA %.1fs", threads, (double)eta_ms * 0.001);
            else
                ImGui::TextDisabled("%d threads  %.0f ms", threads, (double)elapsed_ms);
        } else if (elapsed_ms > 0) {
            ImGui::TextDisabled("%.0f ms", (double)elapsed_ms);
        }
    }
    ImGui::End();
}

void imgui_menu_init(void) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::StyleColorsDark();
    ImGuiStyle &style     = ImGui::GetStyle();
    style.WindowRounding  = 4.f;
    style.WindowBorderSize = 0.f;

    s_navbar.on_level_select = []() { s_level_select.open(); };
    s_navbar.on_physics      = []() { s_physics.toggle(); };
    s_navbar.on_level_debug  = []() { s_level_debug.toggle(); };
    s_navbar.on_shader       = []() { s_shader.toggle(); };

    s_toast.init();
    s_navbar.init();
    s_level_select.init();
    s_level_debug.init();
    s_physics.init();
    s_shader.init();
}

void imgui_menu_shutdown(void) {
    s_shader.shutdown();
    s_physics.shutdown();
    s_level_debug.shutdown();
    s_level_select.shutdown();
    s_navbar.shutdown();
    s_toast.shutdown();
    ImGui::DestroyContext();
}

void imgui_menu_new_frame(void) {
    ImGui::NewFrame();
}

void imgui_menu_compose_frame(void) {
    s_navbar.compose_frame();
    s_level_select.compose_frame();
    s_level_debug.compose_frame();
    s_physics.compose_frame();
    s_shader.compose_frame();
    draw_loading_modal();
    s_toast.compose_frame();
}

void imgui_menu_render(void) {
    ImGui::Render();
}

bool imgui_menu_wants_mouse_capture(void) {
    ImGuiIO &io = ImGui::GetIO();
    return io.WantCaptureMouse                          ||
           ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
           ImGui::IsAnyItemHovered()                    ||
           ImGui::IsAnyItemActive();
}

extern "C" void toast_push(const char *title, const char *body) {
    Toast::push(title, body);
}

#else

void imgui_menu_init(void) {}
void imgui_menu_shutdown(void) {}
void imgui_menu_new_frame(void) {}
void imgui_menu_compose_frame(void) {}
void imgui_menu_render(void) {}
bool imgui_menu_wants_mouse_capture(void) { return false; }
extern "C" void toast_push(const char *, const char *) {}

#endif


