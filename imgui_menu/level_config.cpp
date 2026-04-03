#if defined(ENABLE_DX11) || defined(ENABLE_DX12) || defined(TARGET_LINUX)

#include <cmath>
#include <cfloat>
#include "imgui.h"
#include "level_config.h"

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
extern "C" {
#include "sm64.h"
#include "types.h"
#include "game/main.h"
#include "game/camera.h"
#include "engine/surface_load.h"
#include "pc/gfx/gfx_pc.h"
}

static bool  s_levelConfigVisible = false;
static bool  s_wireframeEnabled   = false;
static float s_wireframeFov       = 45.0f;

static int   s_selectedTexIdx     = -1;
static float s_thumbSize          = 64.0f;

void level_config_toggle(void) {
    s_levelConfigVisible = !s_levelConfigVisible;
}

static void draw_debug_tab(void) {
    ImGui::SeparatorText("Debug");
    {
        bool showProfiler = (gShowProfiler != 0);
        if (ImGui::Checkbox("Show Profiler", &showProfiler))
            gShowProfiler = (s8)showProfiler;

        bool showDebugText = (gShowDebugText != 0);
        if (ImGui::Checkbox("Show Debug Text", &showDebugText))
            gShowDebugText = (s8)showDebugText;
    }

    ImGui::SeparatorText("Rendering");
    ImGui::Checkbox("Wireframe Hitboxes", &s_wireframeEnabled);
    if (s_wireframeEnabled) {
        ImGui::SliderFloat("FOV", &s_wireframeFov, 20.0f, 120.0f);
    }
}

static void draw_textures_tab(void) {
    uint32_t count = gfx_texture_cache_count();

    ImGui::SliderFloat("Thumb Size", &s_thumbSize, 32.0f, 128.0f);
    ImGui::Text("Cached: %u", count);
    ImGui::Separator();

    float panelW = ImGui::GetContentRegionAvail().x;
    float thumbSz = s_thumbSize;
    float pad = ImGui::GetStyle().ItemSpacing.x;
    int cols = (int)((panelW + pad) / (thumbSz + pad));
    if (cols < 1) cols = 1;

    ImGui::BeginChild("##texgrid", ImVec2(0.0f, 300.0f), false);
    for (uint32_t i = 0; i < count; i++) {
        GfxTextureCacheEntry e = gfx_texture_cache_entry(i);
        uintptr_t handle = gfx_get_imgui_tex_id(e.texture_id);
        ImTextureID imgui_id = (ImTextureID)handle;

        int col = (int)(i % (uint32_t)cols);
        if (col != 0) ImGui::SameLine();

        ImGui::PushID((int)i);
        bool selected = (s_selectedTexIdx == (int)i);

        if (handle != 0) {
            if (ImGui::ImageButton("##t", imgui_id, ImVec2(thumbSz, thumbSz),
                                   ImVec2(0,0), ImVec2(1,1),
                                   selected ? ImVec4(0.3f,0.6f,1.0f,1.0f) : ImVec4(0,0,0,0))) {
                s_selectedTexIdx = selected ? -1 : (int)i;
            }
        } else {
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x+thumbSz, p.y+thumbSz),
                                                      selected ? IM_COL32(80,140,255,200) : IM_COL32(60,60,60,200));
            ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x+thumbSz, p.y+thumbSz), IM_COL32(120,120,120,255));
            ImGui::Dummy(ImVec2(thumbSz, thumbSz));
            if (ImGui::IsItemClicked()) s_selectedTexIdx = selected ? -1 : (int)i;
        }

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("#%u  id=%u  %dx%d", i, e.texture_id, (int)e.width, (int)e.height);
            ImGui::EndTooltip();
        }
        ImGui::PopID();
    }
    ImGui::EndChild();

    if (s_selectedTexIdx >= 0 && (uint32_t)s_selectedTexIdx < count) {
        ImGui::Separator();
        GfxTextureCacheEntry e = gfx_texture_cache_entry((uint32_t)s_selectedTexIdx);
        uintptr_t handle = gfx_get_imgui_tex_id(e.texture_id);
        ImGui::Text("Selected: #%d  id=%u  %dx%d", s_selectedTexIdx, e.texture_id, (int)e.width, (int)e.height);

        float avail = ImGui::GetContentRegionAvail().x;
        float previewW = avail < 256.0f ? avail : 256.0f;
        float previewH = (e.width > 0 && e.height > 0)
            ? previewW * ((float)e.height / (float)e.width)
            : previewW;
        if (handle != 0) {
            ImGui::Image((ImTextureID)handle, ImVec2(previewW, previewH));
        } else {
            ImGui::TextDisabled("(no preview on this backend)");
        }
    }
}

static void draw_level_config_window(void) {
    if (!s_levelConfigVisible) return;

    ImGui::SetNextWindowSize(ImVec2(410.0f, 0.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Level Config", &s_levelConfigVisible);

    if (ImGui::BeginTabBar("##lctabs")) {
        if (ImGui::BeginTabItem("Debug")) {
            draw_debug_tab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Textures")) {
            draw_textures_tab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

static void mul4x4(const float a[4][4], const float b[4][4], float out[4][4]) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            float v = 0.0f;
            for (int k = 0; k < 4; k++)
                v += a[r][k] * b[k][c];
            out[r][c] = v;
        }
    }
}

static void build_viewproj(float vp[4][4]) {
    const float *eye   = gLakituState.pos;
    const float *focus = gLakituState.focus;

    float fx = focus[0] - eye[0];
    float fy = focus[1] - eye[1];
    float fz = focus[2] - eye[2];
    float flen = sqrtf(fx*fx + fy*fy + fz*fz);
    if (flen < 1e-6f) flen = 1e-6f;
    fx /= flen; fy /= flen; fz /= flen;

    float rx = fy*0.0f - fz*1.0f;
    float ry = fz*0.0f - fx*0.0f;
    float rz = fx*1.0f - fy*0.0f;
    float rlen = sqrtf(rx*rx + ry*ry + rz*rz);
    if (rlen < 1e-6f) rlen = 1e-6f;
    rx /= rlen; ry /= rlen; rz /= rlen;

    float ux = ry*fz - rz*fy;
    float uy = rz*fx - rx*fz;
    float uz = rx*fy - ry*fx;

    float view[4][4] = {
        { rx,  ry,  rz, -(rx*eye[0] + ry*eye[1] + rz*eye[2]) },
        { ux,  uy,  uz, -(ux*eye[0] + uy*eye[1] + uz*eye[2]) },
        {-fx, -fy, -fz,  (fx*eye[0] + fy*eye[1] + fz*eye[2]) },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    };

    float aspect = (gfx_current_dimensions.height > 0)
        ? (float)gfx_current_dimensions.width / (float)gfx_current_dimensions.height
        : 1.0f;
    float fovRad = s_wireframeFov * 3.14159265358979323846f / 180.0f;
    float tanHalf = tanf(fovRad * 0.5f);
    const float zNear = 100.0f;
    const float zFar  = 12800.0f;

    float proj[4][4] = {
        { 1.0f / (aspect * tanHalf), 0.0f,            0.0f,                            0.0f },
        { 0.0f,                      1.0f / tanHalf,  0.0f,                            0.0f },
        { 0.0f,                      0.0f,           -(zFar + zNear) / (zFar - zNear), -(2.0f * zFar * zNear) / (zFar - zNear) },
        { 0.0f,                      0.0f,           -1.0f,                            0.0f }
    };

    mul4x4(proj, view, vp);
}

static bool project_vertex(const float vp[4][4], float wx, float wy, float wz,
                            float dispW, float dispH, ImVec2 *out) {
    float cx = vp[0][0]*wx + vp[0][1]*wy + vp[0][2]*wz + vp[0][3];
    float cy = vp[1][0]*wx + vp[1][1]*wy + vp[1][2]*wz + vp[1][3];
    float cw = vp[3][0]*wx + vp[3][1]*wy + vp[3][2]*wz + vp[3][3];
    if (cw <= 0.0f) return false;
    float ndcX =  cx / cw;
    float ndcY =  cy / cw;
    out->x = (ndcX * 0.5f + 0.5f) * dispW;
    out->y = (1.0f - (ndcY * 0.5f + 0.5f)) * dispH;
    return true;
}

static void draw_wireframe_surfaces(void) {
    if (!s_wireframeEnabled) return;

    float vp[4][4];
    build_viewproj(vp);

    float dispW = (float)gfx_current_dimensions.width;
    float dispH = (float)gfx_current_dimensions.height;
    ImDrawList *dl = ImGui::GetBackgroundDrawList();

    static const ImU32 kColors[3] = {
        IM_COL32(0,   220,  64,  160),
        IM_COL32(64,  128, 255,  160),
        IM_COL32(255,  64,  64,  160),
    };

    SpatialPartitionCell (*partitions[2])[NUM_CELLS] = {
        gStaticSurfacePartition,
        gDynamicSurfacePartition
    };

    for (int p = 0; p < 2; p++) {
        for (int ci = 0; ci < NUM_CELLS; ci++) {
            for (int cj = 0; cj < NUM_CELLS; cj++) {
                for (int type = 0; type < 3; type++) {
                    ImU32 col = kColors[type];
                    struct SurfaceNode *node = partitions[p][ci][cj][type].next;
                    while (node) {
                        struct Surface *s = node->surface;
                        ImVec2 p0, p1, p2;
                        bool v0 = project_vertex(vp, (float)s->vertex1[0], (float)s->vertex1[1], (float)s->vertex1[2], dispW, dispH, &p0);
                        bool v1 = project_vertex(vp, (float)s->vertex2[0], (float)s->vertex2[1], (float)s->vertex2[2], dispW, dispH, &p1);
                        bool v2 = project_vertex(vp, (float)s->vertex3[0], (float)s->vertex3[1], (float)s->vertex3[2], dispW, dispH, &p2);
                        if (v0 && v1 && v2) {
                            dl->AddLine(p0, p1, col);
                            dl->AddLine(p1, p2, col);
                            dl->AddLine(p2, p0, col);
                        }
                        node = node->next;
                    }
                }
            }
        }
    }
}

void level_config_compose_frame(void) {
    draw_level_config_window();
    draw_wireframe_surfaces();
}

#else

void level_config_toggle(void) {}
void level_config_compose_frame(void) {}

#endif
