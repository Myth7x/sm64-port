#pragma once
#include <cstdint>
#include <cstdio>
#include <functional>
#include "../components/base/component.h"
#include "sm64_udp/manager.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <psapi.h>
#endif

extern "C" const char *level_get_string(void);

class NavBar : public Component {
public:
    std::function<void()> on_level_select;
    std::function<void()> on_physics;
    std::function<void()> on_level_debug;
    std::function<void()> on_shader;

    void init() override {}
    void shutdown() override {}

    void compose_frame() override {
        m_stat_timer += ImGui::GetIO().DeltaTime;
        if (m_stat_timer >= 0.5f) {
            m_stat_timer = 0.f;
            m_cpu_pct    = poll_cpu();
            m_mem_mb     = poll_mem_mb();
        }

        ImGuiIO &io = ImGui::GetIO();
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration       |
            ImGuiWindowFlags_NoSavedSettings    |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav              |
            ImGuiWindowFlags_NoMove             |
            ImGuiWindowFlags_NoResize;

        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 0.f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.82f);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(8.f, 2.f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 3.f));
        ImGui::Begin("##navbar", nullptr, flags);

        char info[192];
        snprintf(info, sizeof(info), "FPS:%.0f  CPU:%.1f%%  MEM:%zuMB  MAP:%s",
                 (double)io.Framerate, (double)m_cpu_pct, m_mem_mb, level_get_string());
        ImGui::TextDisabled("%s", info);

        {
            int         conn = udp_connected();
            const char *st   = udp_status();
            int         users = udp_get_room_user_count();

            ImVec4 badge_col = conn
                ? ImVec4(0.3f, 1.f, 0.3f, 1.f)
                : (st[0] == 'e' ? ImVec4(1.f, 0.35f, 0.35f, 1.f)
                                : ImVec4(1.f, 0.8f, 0.2f, 1.f));

            ImGui::SameLine(0.f, 16.f);
            ImGui::TextColored(badge_col, "[UDP]");
            ImGui::SameLine(0.f, 4.f);
            ImGui::TextDisabled("%s", st);
            if (conn && users > 0) {
                ImGui::SameLine(0.f, 8.f);
                ImGui::TextColored(ImVec4(0.55f, 0.88f, 1.f, 1.f),
                    "%d user%s", users, users == 1 ? "" : "s");
            }
        }

        const float btn_w       = 100.f;
        const float spacing     = ImGui::GetStyle().ItemSpacing.x;
        const float total_btn_w = btn_w * 4.f + spacing * 3.f;
        ImGui::SameLine(ImGui::GetWindowWidth() - total_btn_w - 8.f);

        if (ImGui::Button("Level Select", ImVec2(btn_w, 0.f))) { if (on_level_select) on_level_select(); }
        ImGui::SameLine();
        if (ImGui::Button("Physics",      ImVec2(btn_w, 0.f))) { if (on_physics) on_physics(); }
        ImGui::SameLine();
        if (ImGui::Button("Level Debug",  ImVec2(btn_w, 0.f))) { if (on_level_debug) on_level_debug(); }
        ImGui::SameLine();
        if (ImGui::Button("Shader",       ImVec2(btn_w, 0.f))) { if (on_shader) on_shader(); }

        ImGui::End();
        ImGui::PopStyleVar(2);
    }

private:
    float  m_stat_timer = 0.f;
    float  m_cpu_pct    = 0.f;
    size_t m_mem_mb     = 0;

    static float poll_cpu() {
#ifdef _WIN32
        static FILETIME s_idle = {}, s_kern = {}, s_user = {};
        static float    s_last = 0.f;
        FILETIME idle, kern, user;
        if (!GetSystemTimes(&idle, &kern, &user)) return s_last;
        auto u64 = [](FILETIME ft) -> uint64_t {
            return ((uint64_t)ft.dwHighDateTime << 32) | (uint64_t)ft.dwLowDateTime;
        };
        uint64_t di = u64(idle) - u64(s_idle);
        uint64_t dk = u64(kern) - u64(s_kern);
        uint64_t du = u64(user) - u64(s_user);
        s_idle = idle; s_kern = kern; s_user = user;
        uint64_t tot = dk + du;
        if (tot == 0) return s_last;
        s_last = (float)(tot - di) / (float)tot * 100.f;
        return s_last;
#elif defined(__linux__)
        static long long s_total  = 0, s_idle_v = 0;
        static float     s_last   = 0.f;
        FILE *f = fopen("/proc/stat", "r");
        if (!f) return s_last;
        long long u, n, s, id, wa, hi, si, st;
        fscanf(f, "cpu %lld %lld %lld %lld %lld %lld %lld %lld", &u, &n, &s, &id, &wa, &hi, &si, &st);
        fclose(f);
        long long total = u + n + s + id + wa + hi + si + st;
        long long dt    = total - s_total;
        long long di    = id - s_idle_v;
        s_total = total; s_idle_v = id;
        if (dt > 0) s_last = (float)(dt - di) / (float)dt * 100.f;
        return s_last;
#else
        return 0.f;
#endif
    }

    static size_t poll_mem_mb() {
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS pmc = {};
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
            return (size_t)(pmc.WorkingSetSize >> 20);
        return 0;
#elif defined(__linux__)
        FILE *f = fopen("/proc/self/status", "r");
        if (!f) return 0;
        char  line[128];
        long long kb = 0;
        while (fgets(line, sizeof(line), f))
            if (sscanf(line, "VmRSS: %lld kB", &kb) == 1) break;
        fclose(f);
        return (size_t)(kb >> 10);
#else
        return 0;
#endif
    }
};
