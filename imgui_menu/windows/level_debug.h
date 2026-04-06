#pragma once
#include <cmath>
#include <cfloat>
#include "../components/window.h"

extern "C" {
    extern int8_t  gShowProfiler;
    extern int8_t  gShowDebugText;
    #include "game/entity_boxes.h"
}

class LevelDebug : public Window {
public:
    LevelDebug() : Window("Level Debug", ImVec2(410.f, 0.f)) {}

    void compose_frame() override {
        draw_wireframe_surfaces();
        draw_entity_boxes();
        Window::compose_frame();
    }

protected:
    void draw_content() override {
        if (ImGui::BeginTabBar("##ldtabs")) {
            if (ImGui::BeginTabItem("Debug"))    { tab_debug();    ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Textures")) { tab_textures(); ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Entities")) { tab_entities(); ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }
    }

private:
    bool  m_wireframe       = false;
    int   m_sel_tex         = -1;
    float m_thumb_sz        = 64.f;
    bool  m_show_entities   = false;
    bool  m_ebox_filter[8]  = {false, true, true, true, true, true, true, true};

    void tab_debug() {
        ImGui::SeparatorText("Debug");
        bool show_prof = (gShowProfiler != 0);
        if (ImGui::Checkbox("Show Profiler",    &show_prof))  gShowProfiler  = (int8_t)show_prof;
        bool show_dbg  = (gShowDebugText != 0);
        if (ImGui::Checkbox("Show Debug Text",  &show_dbg))   gShowDebugText = (int8_t)show_dbg;

        ImGui::SeparatorText("Rendering");
        ImGui::Checkbox("Wireframe Hitboxes", &m_wireframe);
    }

    void tab_textures() {
        uint32_t count = gfx_texture_cache_count();
        ImGui::SliderFloat("Thumb Size", &m_thumb_sz, 32.f, 128.f);
        ImGui::Text("Cached: %u", count);
        ImGui::Separator();

        float panel_w = ImGui::GetContentRegionAvail().x;
        float pad     = ImGui::GetStyle().ItemSpacing.x;
        int   cols    = (int)((panel_w + pad) / (m_thumb_sz + pad));
        if (cols < 1) cols = 1;

        ImGui::BeginChild("##texgrid", ImVec2(0.f, 300.f), false);
        for (uint32_t i = 0; i < count; i++) {
            GfxTextureCacheEntry e  = gfx_texture_cache_entry(i);
            uintptr_t            hd = gfx_get_imgui_tex_id(e.texture_id);
            ImTextureID          id = (ImTextureID)hd;

            if ((int)(i % (uint32_t)cols) != 0) ImGui::SameLine();

            ImGui::PushID((int)i);
            bool sel = (m_sel_tex == (int)i);
            if (hd != 0) {
                if (ImGui::ImageButton("##t", id, ImVec2(m_thumb_sz, m_thumb_sz),
                        ImVec2(0,0), ImVec2(1,1),
                        sel ? ImVec4(0.3f,0.6f,1.f,1.f) : ImVec4(0,0,0,0)))
                    m_sel_tex = sel ? -1 : (int)i;
            } else {
                ImVec2 p = ImGui::GetCursorScreenPos();
                ImGui::GetWindowDrawList()->AddRectFilled(p,
                    ImVec2(p.x + m_thumb_sz, p.y + m_thumb_sz),
                    sel ? IM_COL32(80,140,255,200) : IM_COL32(60,60,60,200));
                ImGui::GetWindowDrawList()->AddRect(p,
                    ImVec2(p.x + m_thumb_sz, p.y + m_thumb_sz), IM_COL32(120,120,120,255));
                ImGui::Dummy(ImVec2(m_thumb_sz, m_thumb_sz));
                if (ImGui::IsItemClicked()) m_sel_tex = sel ? -1 : (int)i;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("#%u  id=%u  %dx%d", i, e.texture_id, (int)e.width, (int)e.height);
                ImGui::EndTooltip();
            }
            ImGui::PopID();
        }
        ImGui::EndChild();

        if (m_sel_tex >= 0 && (uint32_t)m_sel_tex < count) {
            ImGui::Separator();
            GfxTextureCacheEntry e  = gfx_texture_cache_entry((uint32_t)m_sel_tex);
            uintptr_t            hd = gfx_get_imgui_tex_id(e.texture_id);
            ImGui::Text("Selected: #%d  id=%u  %dx%d", m_sel_tex, e.texture_id, (int)e.width, (int)e.height);
            float avail    = ImGui::GetContentRegionAvail().x;
            float preview_w = avail < 256.f ? avail : 256.f;
            float preview_h = (e.width > 0 && e.height > 0)
                ? preview_w * ((float)e.height / (float)e.width) : preview_w;
            if (hd != 0)
                ImGui::Image((ImTextureID)hd, ImVec2(preview_w, preview_h));
            else
                ImGui::TextDisabled("(no preview on this backend)");
        }
    }

    static void mul4x4(const float a[4][4], const float b[4][4], float out[4][4]) {
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++) {
                float v = 0.f;
                for (int k = 0; k < 4; k++) v += a[r][k] * b[k][c];
                out[r][c] = v;
            }
    }

    void build_viewproj(float vp[4][4]) const {
        const float *eye   = gLakituState.pos;
        const float *focus = gLakituState.focus;
        float fx = focus[0]-eye[0], fy = focus[1]-eye[1], fz = focus[2]-eye[2];
        float fl = sqrtf(fx*fx + fy*fy + fz*fz); if (fl < 1e-6f) fl = 1e-6f;
        fx /= fl; fy /= fl; fz /= fl;

        float rx = fy*0.f - fz*1.f, ry = fz*0.f - fx*0.f, rz = fx*1.f - fy*0.f;
        float rl = sqrtf(rx*rx + ry*ry + rz*rz); if (rl < 1e-6f) rl = 1e-6f;
        rx /= rl; ry /= rl; rz /= rl;

        float ux = ry*fz - rz*fy, uy = rz*fx - rx*fz, uz = rx*fy - ry*fx;

        float view[4][4] = {
            { rx,  ry,  rz, -(rx*eye[0]+ry*eye[1]+rz*eye[2]) },
            { ux,  uy,  uz, -(ux*eye[0]+uy*eye[1]+uz*eye[2]) },
            {-fx, -fy, -fz,  (fx*eye[0]+fy*eye[1]+fz*eye[2]) },
            { 0.f, 0.f, 0.f, 1.f }
        };

        float aspect = (gfx_current_dimensions.height > 0)
            ? (float)gfx_current_dimensions.width / (float)gfx_current_dimensions.height : 1.f;
        float fov_rad  = 65.f * 3.14159265358979323846f / 180.f;
        float tan_half = tanf(fov_rad * 0.5f);
        const float zN = 100.f, zF = 12800.f;

        float proj[4][4] = {
            { 1.f/(aspect*tan_half), 0.f,          0.f,                        0.f },
            { 0.f,                   1.f/tan_half,  0.f,                        0.f },
            { 0.f,                   0.f,          -(zF+zN)/(zF-zN),           -(2.f*zF*zN)/(zF-zN) },
            { 0.f,                   0.f,          -1.f,                        0.f }
        };
        mul4x4(proj, view, vp);
    }

    static bool project_vtx(const float vp[4][4], float wx, float wy, float wz,
                             float dw, float dh, ImVec2 *out) {
        float cx = vp[0][0]*wx + vp[0][1]*wy + vp[0][2]*wz + vp[0][3];
        float cy = vp[1][0]*wx + vp[1][1]*wy + vp[1][2]*wz + vp[1][3];
        float cw = vp[3][0]*wx + vp[3][1]*wy + vp[3][2]*wz + vp[3][3];
        if (cw <= 0.f) return false;
        out->x = ( cx/cw *  0.5f + 0.5f) * dw;
        out->y = (-cy/cw * 0.5f + 0.5f) * dh;
        return true;
    }

    void draw_wireframe_surfaces() {
        if (!m_wireframe) return;
        float vp[4][4];
        build_viewproj(vp);
        float dw = (float)gfx_current_dimensions.width;
        float dh = (float)gfx_current_dimensions.height;
        ImDrawList *dl = ImGui::GetBackgroundDrawList();

        static const ImU32 kCol[3] = {
            IM_COL32(0,  220, 64,  160),
            IM_COL32(64, 128, 255, 160),
            IM_COL32(255, 64, 64,  160),
        };
        SpatialPartitionCell (*parts[2])[NUM_CELLS] = {
            gStaticSurfacePartition, gDynamicSurfacePartition
        };
        for (int p = 0; p < 2; p++)
            for (int ci = 0; ci < NUM_CELLS; ci++)
                for (int cj = 0; cj < NUM_CELLS; cj++)
                    for (int t = 0; t < 3; t++) {
                        struct SurfaceNode *node = parts[p][ci][cj][t].next;
                        while (node) {
                            struct Surface *s = node->surface;
                            ImVec2 p0, p1, p2;
                            bool v0 = project_vtx(vp,(float)s->vertex1[0],(float)s->vertex1[1],(float)s->vertex1[2],dw,dh,&p0);
                            bool v1 = project_vtx(vp,(float)s->vertex2[0],(float)s->vertex2[1],(float)s->vertex2[2],dw,dh,&p1);
                            bool v2 = project_vtx(vp,(float)s->vertex3[0],(float)s->vertex3[1],(float)s->vertex3[2],dw,dh,&p2);
                            if (v0 && v1 && v2) {
                                dl->AddLine(p0,p1,kCol[t]);
                                dl->AddLine(p1,p2,kCol[t]);
                                dl->AddLine(p2,p0,kCol[t]);
                            }
                            node = node->next;
                        }
                    }
    }

    static void draw_aabb_box(ImDrawList *dl, const float vp[4][4],
                              float dw, float dh,
                              float x0, float y0, float z0,
                              float x1, float y1, float z1,
                              ImU32 col) {
        static const int edges[12][2] = {
            {0,1},{1,2},{2,3},{3,0},
            {4,5},{5,6},{6,7},{7,4},
            {0,4},{1,5},{2,6},{3,7},
        };
        float cx[8] = {x0,x1,x1,x0, x0,x1,x1,x0};
        float cy[8] = {y0,y0,y0,y0, y1,y1,y1,y1};
        float cz[8] = {z0,z0,z1,z1, z0,z0,z1,z1};
        ImVec2 pts[8];
        bool ok[8];
        for (int i = 0; i < 8; i++)
            ok[i] = project_vtx(vp, cx[i], cy[i], cz[i], dw, dh, &pts[i]);
        for (int e = 0; e < 12; e++) {
            int a = edges[e][0], b = edges[e][1];
            if (ok[a] && ok[b])
                dl->AddLine(pts[a], pts[b], col);
        }
    }

    void draw_entity_boxes() {
        if (!m_show_entities) return;
        const struct EntityBox *boxes = entity_boxes_get();
        if (!boxes) return;

        static const ImU32 kTypeCol[8] = {
            IM_COL32(128,128,128,180),
            IM_COL32(255,  60, 60,200),
            IM_COL32( 60,128,255,200),
            IM_COL32(255,220, 60,200),
            IM_COL32(  0,200,120,200),
            IM_COL32( 60,220, 60,200),
            IM_COL32(255,140,  0,200),
            IM_COL32(180,180,180,200),
        };

        float vp[4][4];
        build_viewproj(vp);
        float dw = (float)gfx_current_dimensions.width;
        float dh = (float)gfx_current_dimensions.height;
        ImDrawList *dl = ImGui::GetBackgroundDrawList();

        static const char *kTypeLabel[8] = {
            "none","Death","Teleport","Script","Door","Brush","Logic","Landmark"
        };

        for (const struct EntityBox *b = boxes; b->type != 0; ++b) {
            int t = b->type;
            if (t < 0 || t > 7) t = 0;
            if (!m_ebox_filter[t]) continue;
            float x0 = (float)b->mins[0], y0 = (float)b->mins[1], z0 = (float)b->mins[2];
            float x1 = (float)b->maxs[0], y1 = (float)b->maxs[1], z1 = (float)b->maxs[2];
            draw_aabb_box(dl, vp, dw, dh, x0, y0, z0, x1, y1, z1, kTypeCol[t]);

            ImVec2 ctr;
            if (project_vtx(vp, (x0+x1)*0.5f, (y0+y1)*0.5f, (z0+z1)*0.5f, dw, dh, &ctr)) {
                char buf[96];
                snprintf(buf, sizeof(buf), "%s\n(%d,%d,%d)\n(%d,%d,%d)",
                    kTypeLabel[t],
                    (int)x0,(int)y0,(int)z0,
                    (int)x1,(int)y1,(int)z1);
                ImVec2 tsz = ImGui::CalcTextSize(buf);
                ImVec2 tpos = ImVec2(ctr.x - tsz.x * 0.5f, ctr.y - tsz.y * 0.5f);
                dl->AddRectFilled(
                    ImVec2(tpos.x - 2.f, tpos.y - 2.f),
                    ImVec2(tpos.x + tsz.x + 2.f, tpos.y + tsz.y + 2.f),
                    IM_COL32(0, 0, 0, 160));
                dl->AddText(tpos, kTypeCol[t], buf);
            }
        }
    }

    void tab_entities() {
        static const char *kTypeLabels[8] = {
            "none", "Death", "Teleport", "Script", "Door", "Brush", "Logic", "Landmark"
        };
        static const ImU32 kTypeCol[8] = {
            IM_COL32(128,128,128,255),
            IM_COL32(255, 60, 60,255),
            IM_COL32( 60,128,255,255),
            IM_COL32(255,220, 60,255),
            IM_COL32(  0,200,120,255),
            IM_COL32( 60,220, 60,255),
            IM_COL32(255,140,  0,255),
            IM_COL32(180,180,180,255),
        };

        ImGui::Checkbox("Show Entity Boxes", &m_show_entities);
        if (!m_show_entities) return;
        ImGui::SeparatorText("Filters");
        for (int i = 1; i < 8; i++) {
            ImGui::PushID(i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(
                p, ImVec2(p.x + 12.f, p.y + 12.f), kTypeCol[i]);
            ImGui::Dummy(ImVec2(14.f, 12.f));
            ImGui::SameLine();
            ImGui::Checkbox(kTypeLabels[i], &m_ebox_filter[i]);
            ImGui::PopID();
        }
        const struct EntityBox *boxes = entity_boxes_get();
        if (!boxes) { ImGui::TextDisabled("No entity data loaded"); return; }
        ImGui::SeparatorText("Boxes");
        int count = 0;
        for (const struct EntityBox *b = boxes; b->type != 0; ++b) count++;
        ImGui::Text("Total: %d", count);
    }
};

