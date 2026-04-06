#pragma once
#include <vector>
#include <deque>
#include <mutex>
#include <algorithm>
#include <cstring>
#include "base/component.h"

class Toast : public Component {
public:
    static void push(const char *title, const char *body) {
        PendingEntry p = {};
        if (title) strncpy(p.title, title, sizeof(p.title) - 1);
        if (body)  strncpy(p.body,  body,  sizeof(p.body)  - 1);
        std::lock_guard<std::mutex> lk(s_mutex());
        s_pending().push_back(p);
    }

    void init() override {}
    void shutdown() override {}

    void compose_frame() override {
        {
            std::lock_guard<std::mutex> lk(s_mutex());
            while (!s_pending().empty()) {
                auto &active = s_active();
                if ((int)active.size() >= k_max)
                    active.erase(active.begin());
                const PendingEntry &p = s_pending().front();
                ActiveEntry e = {};
                strncpy(e.title, p.title, sizeof(e.title) - 1);
                strncpy(e.body,  p.body,  sizeof(e.body)  - 1);
                active.push_back(e);
                s_pending().pop_front();
            }
        }
        auto &active = s_active();
        if (active.empty()) return;

        const float dt     = ImGui::GetIO().DeltaTime;
        const float scrn_x = ImGui::GetIO().DisplaySize.x;
        if (scrn_x <= 0.f) return;

        const int   n    = (int)active.size();
        ImDrawList *dl   = ImGui::GetForegroundDrawList();
        ImFont     *font = ImGui::GetFont();
        const float fsz  = ImGui::GetFontSize();
        const float pad  = 8.f;

        for (int i = n - 1; i >= 0; i--) {
            ActiveEntry &e = active[i];
            e.age += dt;
            float tgt_y = k_margin + (float)(n - 1 - i) * (k_h + k_gap);
            if (!e.anim_init) { e.anim_y = tgt_y - 20.f; e.anim_init = true; }
            e.anim_y += (tgt_y - e.anim_y) * std::min(1.f, dt * 10.f);

            float alpha = 1.f;
            if (e.age > k_show_sec)
                alpha = 1.f - (e.age - k_show_sec) / k_fade_sec;
            if (alpha <= 0.f) continue;

            const float x0 = scrn_x - k_w - k_margin;
            const float y0 = e.anim_y;
            dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x0 + k_w, y0 + k_h),
                ImGui::ColorConvertFloat4ToU32(ImVec4(0.07f, 0.07f, 0.10f, alpha * 0.88f)), 8.f);
            dl->PushClipRect(ImVec2(x0, y0), ImVec2(x0 + k_w, y0 + k_h), true);
            dl->AddText(font, fsz, ImVec2(x0 + pad, y0 + pad),
                ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, alpha)), e.title);
            dl->AddText(font, fsz, ImVec2(x0 + pad, y0 + pad + fsz + 4.f),
                ImGui::ColorConvertFloat4ToU32(ImVec4(0.75f, 0.75f, 0.75f, alpha)), e.body,
                nullptr, k_w - pad * 2.f);
            dl->PopClipRect();
        }
        active.erase(
            std::remove_if(active.begin(), active.end(),
                [](const ActiveEntry &e) { return e.age >= k_show_sec + k_fade_sec; }),
            active.end());
    }

private:
    static constexpr int   k_max      = 5;
    static constexpr float k_w        = 280.f;
    static constexpr float k_h        = 64.f;
    static constexpr float k_gap      = 8.f;
    static constexpr float k_margin   = 12.f;
    static constexpr float k_show_sec = 3.f;
    static constexpr float k_fade_sec = 0.5f;

    struct PendingEntry { char title[64]; char body[128]; };
    struct ActiveEntry {
        char  title[64]  = {};
        char  body[128]  = {};
        float age        = 0.f;
        float anim_y     = 0.f;
        bool  anim_init  = false;
    };

    static std::vector<ActiveEntry>  &s_active()  { static std::vector<ActiveEntry>  v; return v; }
    static std::deque<PendingEntry>  &s_pending() { static std::deque<PendingEntry>  d; return d; }
    static std::mutex                &s_mutex()   { static std::mutex m; return m; }
};