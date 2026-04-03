#if defined(ENABLE_DX11) || defined(ENABLE_DX12) || defined(TARGET_LINUX)

#include "imgui.h"
#include "physics_config.h"

extern "C" {
#include "src/game/source_movement.h"
}

static bool s_physicsWindowVisible = false;

void physics_config_toggle(void) {
    s_physicsWindowVisible = !s_physicsWindowVisible;
}

void physics_config_compose_frame(void) {
    if (!s_physicsWindowVisible) return;

    ImGui::SetNextWindowSize(ImVec2(340.0f, 0.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Physics Config", &s_physicsWindowVisible);

    ImGui::SeparatorText("Ground");
    ImGui::SliderFloat("Max Speed",        &gSrcMaxSpeed,      1.0f,  200.0f);
    ImGui::SliderFloat("Acceleration",     &gSrcAccel,         0.1f,  50.0f);
    ImGui::SliderFloat("Friction",         &gSrcFriction,      0.0f,  20.0f);
    ImGui::SliderFloat("Stop Speed",       &gSrcStopSpeed,     0.0f,  50.0f);
    ImGui::SliderFloat("Jump Speed",       &gSrcJumpSpeed,     5.0f,  150.0f);

    ImGui::SeparatorText("Air");
    ImGui::SliderFloat("Air Accel",        &gSrcAirAccel,      0.1f,  50.0f);
    ImGui::SliderFloat("Air Wish Cap",     &gSrcAirWishCap,    0.1f,  50.0f);
    ImGui::SliderFloat("Extra Gravity",    &gSrcExtraGravity,  0.0f,  20.0f);
    ImGui::SliderFloat("Terminal Vel",     &gSrcTerminalVel,  -400.0f, -1.0f);

    ImGui::SeparatorText("Bhop");
    ImGui::SliderFloat("Bhop Speed Cap",   &gSrcBhopSpeedCap,  10.0f, 500.0f);
    ImGui::SliderInt  ("Bhop Window",      &gSrcBhopWindow,    0,     30);

    ImGui::SeparatorText("Noclip");
    ImGui::SliderFloat("Noclip Speed",     &gSrcNoclipSpeed,   1.0f,  200.0f);

    ImGui::SeparatorText("Surface");
    ImGui::SliderFloat("Walkable Normal",  &gSrcWalkableNormal, 0.0f, 1.0f);
    ImGui::SliderFloat("Surf Min Normal",  &gSrcSurfMinNormal,  0.0f, 1.0f);

    if (ImGui::Button("Reset Defaults")) {
        gSrcMaxSpeed       = 24.0f;
        gSrcAccel          = 10.0f;
        gSrcAirAccel       = 10.0f;
        gSrcAirWishCap     = 6.0f;
        gSrcFriction       = 2.5f;
        gSrcStopSpeed      = 8.0f;
        gSrcJumpSpeed      = 42.0f;
        gSrcBhopSpeedCap   = 120.0f;
        gSrcWalkableNormal = 0.7f;
        gSrcSurfMinNormal  = 0.05f;
        gSrcExtraGravity   = 0.8f;
        gSrcTerminalVel    = -160.0f;
        gSrcNoclipSpeed    = 40.0f;
        gSrcBhopWindow     = 4;
    }

    ImGui::End();
}

#else

void physics_config_toggle(void) {}
void physics_config_compose_frame(void) {}

#endif
