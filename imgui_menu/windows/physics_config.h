#pragma once
#include "../components/window.h"

extern "C" {
    extern float gSrcMaxSpeed;
    extern float gSrcAccel;
    extern float gSrcAirAccel;
    extern float gSrcAirWishCap;
    extern float gSrcFriction;
    extern float gSrcStopSpeed;
    extern float gSrcJumpSpeed;
    extern float gSrcBhopSpeedCap;
    extern float gSrcWalkableNormal;
    extern float gSrcSurfMinNormal;
    extern float gSrcExtraGravity;
    extern float gSrcTerminalVel;
    extern float gSrcNoclipSpeed;
    extern int   gSrcBhopWindow;
}

class PhysicsConfig : public Window {
public:
    PhysicsConfig() : Window("Physics Config", ImVec2(340.f, 0.f)) {}

protected:
    void draw_content() override {
        ImGui::SeparatorText("Ground");
        ImGui::SliderFloat("Max Speed",       &gSrcMaxSpeed,       1.f,   200.f);
        ImGui::SliderFloat("Acceleration",    &gSrcAccel,          0.1f,  50.f);
        ImGui::SliderFloat("Friction",        &gSrcFriction,       0.f,   20.f);
        ImGui::SliderFloat("Stop Speed",      &gSrcStopSpeed,      0.f,   50.f);
        ImGui::SliderFloat("Jump Speed",      &gSrcJumpSpeed,      5.f,   150.f);

        ImGui::SeparatorText("Air");
        ImGui::SliderFloat("Air Accel",       &gSrcAirAccel,       0.1f,  50.f);
        ImGui::SliderFloat("Air Wish Cap",    &gSrcAirWishCap,     0.1f,  50.f);
        ImGui::SliderFloat("Extra Gravity",   &gSrcExtraGravity,   0.f,   20.f);
        ImGui::SliderFloat("Terminal Vel",    &gSrcTerminalVel,   -400.f, -1.f);

        ImGui::SeparatorText("Bhop");
        ImGui::SliderFloat("Bhop Speed Cap",  &gSrcBhopSpeedCap,   10.f,  500.f);
        ImGui::SliderInt  ("Bhop Window",     &gSrcBhopWindow,     0,     30);

        ImGui::SeparatorText("Noclip");
        ImGui::SliderFloat("Noclip Speed",    &gSrcNoclipSpeed,    1.f,   200.f);

        ImGui::SeparatorText("Surface");
        ImGui::SliderFloat("Walkable Normal", &gSrcWalkableNormal, 0.f,   1.f);
        ImGui::SliderFloat("Surf Min Normal", &gSrcSurfMinNormal,  0.f,   1.f);

        if (ImGui::Button("Reset Defaults")) {
            gSrcMaxSpeed       = 24.f;
            gSrcAccel          = 10.f;
            gSrcAirAccel       = 10.f;
            gSrcAirWishCap     = 6.f;
            gSrcFriction       = 2.5f;
            gSrcStopSpeed      = 8.f;
            gSrcJumpSpeed      = 42.f;
            gSrcBhopSpeedCap   = 120.f;
            gSrcWalkableNormal = 0.7f;
            gSrcSurfMinNormal  = 0.05f;
            gSrcExtraGravity   = 0.8f;
            gSrcTerminalVel    = -160.f;
            gSrcNoclipSpeed    = 40.f;
            gSrcBhopWindow     = 4;
        }
    }
};
