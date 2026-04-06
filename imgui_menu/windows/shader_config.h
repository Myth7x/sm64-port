#pragma once
#include "../components/window.h"

extern "C" {
#include "pc/gfx/gfx_pc.h"
#include "pc/gfx/gfx_cc.h"
#include "pc/gfx/level_lights.h"
#include "pc/gfx/gfx_direct3d_common.h"
#include "game/level_update.h"
#include "include/types.h"
}

class ShaderConfig : public Window {
public:
    ShaderConfig() : Window("Shader Config", ImVec2(480.f, 560.f)) {}

protected:
    void draw_content() override {
        if (!ImGui::BeginTabBar("##sc_tabs"))
            return;

        if (ImGui::BeginTabItem("Shaders")) {
            draw_shaders_tab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Lights")) {
            draw_lights_tab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("SSAO")) {
            draw_ssao_tab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

private:
    int   m_selected = -1;
    float m_saved_intensity[LEVEL_LIGHTS_MAX] = {};
    bool  m_light_enabled[LEVEL_LIGHTS_MAX]   = { true,true,true,true,true,true,true,true };

    void draw_shaders_tab() {
        uint32_t pool = gfx_combiner_pool_count();

        ImGui::Text("Draw calls: %u  Tris: %u  Shaders: %u",
                    gfx_dbg_draw_calls, gfx_dbg_tris, pool);
        ImGui::Separator();

        static const char *k_cc_name[] = { "0","TEXEL0","TEXEL1","PRIM","SHADE","ENV","TEXEL0A","LOD" };

        if (pool == 0) {
            ImGui::TextDisabled("No shaders compiled yet.");
            return;
        }

        float list_h = ImGui::GetContentRegionAvail().y * 0.38f;
        ImGui::BeginChild("##shader_list", ImVec2(0.f, list_h), true);
        for (uint32_t i = 0; i < pool; i++) {
            uint32_t cc_id = gfx_combiner_pool_cc_id(i);
            char label[32];
            snprintf(label, sizeof(label), "[%3u] 0x%08X", i, cc_id);
            bool sel = (m_selected == (int)i);
            if (ImGui::Selectable(label, sel))
                m_selected = (int)i;
            if (sel) ImGui::SetItemDefaultFocus();

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                CCFeatures f{};
                gfx_cc_get_features(cc_id, &f);
                ImGui::Text("RGB:  (%s - %s) * %s + %s",
                    k_cc_name[f.c[0][0]], k_cc_name[f.c[0][1]],
                    k_cc_name[f.c[0][2]], k_cc_name[f.c[0][3]]);
                ImGui::Text("Alpha:(%s - %s) * %s + %s",
                    k_cc_name[f.c[1][0]], k_cc_name[f.c[1][1]],
                    k_cc_name[f.c[1][2]], k_cc_name[f.c[1][3]]);
                ImGui::EndTooltip();
            }
        }
        ImGui::EndChild();

        if (m_selected >= 0 && (uint32_t)m_selected < pool) {
            ImGui::Spacing();
            uint32_t cc_id = gfx_combiner_pool_cc_id((uint32_t)m_selected);
            CCFeatures f{};
            gfx_cc_get_features(cc_id, &f);

            ImGui::Text("CC ID: 0x%08X", cc_id);
            ImGui::Separator();

            if (ImGui::BeginTable("##ccinfo", 2, ImGuiTableFlags_SizingStretchSame)) {
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("RGB combiner");
                ImGui::Text("  A = %s", k_cc_name[f.c[0][0]]);
                ImGui::Text("  B = %s", k_cc_name[f.c[0][1]]);
                ImGui::Text("  C = %s", k_cc_name[f.c[0][2]]);
                ImGui::Text("  D = %s", k_cc_name[f.c[0][3]]);

                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Alpha combiner");
                ImGui::Text("  A = %s", k_cc_name[f.c[1][0]]);
                ImGui::Text("  B = %s", k_cc_name[f.c[1][1]]);
                ImGui::Text("  C = %s", k_cc_name[f.c[1][2]]);
                ImGui::Text("  D = %s", k_cc_name[f.c[1][3]]);

                ImGui::EndTable();
            }

            ImGui::Separator();
            ImGui::Text("Flags:  alpha=%d  fog=%d  tex_edge=%d  noise=%d",
                (int)f.opt_alpha, (int)f.opt_fog,
                (int)f.opt_texture_edge, (int)f.opt_noise);
            ImGui::Text("Tex:    TEX0=%d  TEX1=%d   inputs=%d  col_a_same=%d",
                (int)f.used_textures[0], (int)f.used_textures[1],
                f.num_inputs, (int)f.color_alpha_same);
            ImGui::Text("Mode:   single=%d/%d  mul=%d/%d  mix=%d/%d",
                (int)f.do_single[0], (int)f.do_single[1],
                (int)f.do_multiply[0], (int)f.do_multiply[1],
                (int)f.do_mix[0], (int)f.do_mix[1]);
        }
    }

    void draw_lights_tab() {
        {
            int ov = level_lights_get_override();
            bool ovb = ov != 0;
            if (ImGui::Checkbox("Override level lighting", &ovb))
                level_lights_set_override(ovb ? 1 : 0);
            if (ovb) {
                ImGui::SameLine();
                ImGui::TextDisabled("(level fn disabled - edits persist)");
            } else {
                ImGui::SameLine();
                ImGui::TextDisabled("(live - resets each frame)");
            }
            ImGui::Separator();
        }

        LightsCBData cb{};
        level_lights_fill_cb(&cb);
        int n = level_lights_count();

        if (ImGui::CollapsingHeader("Ambient", ImGuiTreeNodeFlags_DefaultOpen)) {
            float col[3] = { cb.ambient_color[0], cb.ambient_color[1], cb.ambient_color[2] };
            float intens  = cb.ambient_color[3];
            bool ch = false;
            ch |= ImGui::ColorEdit3("Color##amb", col);
            ch |= ImGui::DragFloat("Intensity##amb", &intens, 0.005f, 0.0f, 4.0f);
            if (ch) level_lights_set_ambient(col[0], col[1], col[2], intens);
        }

        if (ImGui::CollapsingHeader("Sun", ImGuiTreeNodeFlags_DefaultOpen)) {
            float dir[3] = { cb.sun_dir[0], cb.sun_dir[1], cb.sun_dir[2] };
            float col[3] = { cb.sun_color[0], cb.sun_color[1], cb.sun_color[2] };
            bool ch = false;
            ch |= ImGui::DragFloat3("Dir##sun", dir, 0.01f, -1.0f, 1.0f);
            ch |= ImGui::ColorEdit3("Color##sun", col);
            if (ch) level_lights_set_sun(dir[0], dir[1], dir[2], col[0], col[1], col[2]);
        }

        FogCBData fog{};
        level_lights_fill_fog_cb(&fog);
        if (ImGui::CollapsingHeader("Fog")) {
            float fc[3] = { fog.fog_color[0], fog.fog_color[1], fog.fog_color[2] };
            bool ch = false;
            ch |= ImGui::ColorEdit3("Color##fog", fc);
            ch |= ImGui::DragFloat("Near##fog", &fog.fog_near, 50.0f, 0.0f, 1000000.0f);
            ch |= ImGui::DragFloat("Far##fog", &fog.fog_far, 50.0f, 0.0f, 1000000.0f);
            ch |= ImGui::DragFloat("Max Density##fog", &fog.fog_max_density, 0.01f, 0.0f, 1.0f);
            if (ch) level_lights_set_fog(fc[0], fc[1], fc[2], fog.fog_near, fog.fog_far, fog.fog_max_density);
        }

        ImGui::Separator();
        ImGui::Text("Point lights: %d / %d", n, LEVEL_LIGHTS_MAX);

        if (n == 0) {
            ImGui::Spacing();
            ImGui::TextDisabled("No point lights active.");
            return;
        }

        ImGui::Spacing();
        for (int i = 0; i < n; i++) {
            LevelLight pl{};
            level_lights_get(i, &pl);

            char hdr[64];
            snprintf(hdr, sizeof(hdr), "[%d] (%.0f, %.0f, %.0f)  r=%.0f###pl%d",
                     i, pl.position[0], pl.position[1], pl.position[2], pl.radius, i);

            if (ImGui::CollapsingHeader(hdr)) {
                ImGui::PushID(i);
                bool changed = false;

                bool en = m_light_enabled[i];
                if (ImGui::Checkbox("Enabled", &en)) {
                    if (!en) {
                        m_saved_intensity[i] = pl.intensity;
                        pl.intensity = 0.0f;
                    } else {
                        pl.intensity = m_saved_intensity[i] > 0.0f ? m_saved_intensity[i] : 1.0f;
                    }
                    m_light_enabled[i] = en;
                    changed = true;
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Teleport")) {
                    if (gMarioState && gMarioState->marioObj) {
                        gMarioState->pos[0] = pl.position[0];
                        gMarioState->pos[1] = pl.position[1];
                        gMarioState->pos[2] = pl.position[2];
                        gMarioState->marioObj->oPosX = pl.position[0];
                        gMarioState->marioObj->oPosY = pl.position[1];
                        gMarioState->marioObj->oPosZ = pl.position[2];
                    }
                }

                changed |= ImGui::DragFloat3("Position", pl.position, 10.0f);
                changed |= ImGui::DragFloat("Radius", &pl.radius, 10.0f, 0.0f, 100000.0f);
                changed |= ImGui::ColorEdit3("Color", pl.color);
                changed |= ImGui::DragFloat("Intensity", &pl.intensity, 0.01f, 0.0f, 10.0f);

                if (changed) level_lights_set(i, &pl);
                ImGui::PopID();
            }
        }
    }

    void draw_ssao_tab() {
        GfxSSAOParams *p = gfx_ssao_params();
        if (!p) {
            ImGui::TextDisabled("SSAO not available (DX11 only).");
            return;
        }

        ImGui::SeparatorText("SSAO");
        ImGui::Checkbox("Enabled", &p->enabled);
        ImGui::SameLine();
        ImGui::Checkbox("Visualize AO buffer", &p->visualize);
        if (p->visualize && !p->enabled) {
            p->enabled = true;
        }

        ImGui::Spacing();
        ImGui::BeginDisabled(!p->enabled);

        ImGui::DragFloat("Radius", &p->radius, 5.0f, 1.0f, 5000.0f, "%.1f");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Sample hemisphere radius in world units.\nLarger = wider occlusion, more halo at edges.");

        ImGui::DragFloat("Intensity", &p->intensity, 0.05f, 0.0f, 20.0f, "%.2f");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Occlusion multiplier. Higher = darker creases.");

        if (ImGui::Button("Reset##ssao")) {
            p->radius    = 200.0f;
            p->intensity = 1.5f;
        }

        ImGui::EndDisabled();

        ImGui::Spacing();
        ImGui::SeparatorText("Info");
        ImGui::Text("Kernel samples : 12");
        ImGui::Text("Technique      : hemisphere SSAO (view-space)");
        ImGui::Text("Bias           : dynamic  abs(z)*0.001 + 0.5");
        ImGui::Text("Composite blend: multiply (darkens occluded areas)");
        ImGui::Text("Depth source   : R32_FLOAT depth texture");
        ImGui::Text("Resolution     : full (no downscale)");
        if (p->visualize) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1,0.8f,0.2f,1), "Visualize ON: AO map replaces scene.");
        }
    }
};
