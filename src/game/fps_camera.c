#include <ultra64.h>
#include <math.h>

#include "sm64.h"
#include "types.h"
#include "camera.h"
#include "engine/math_util.h"
#include "level_update.h"
#include "fps_mode.h"
#include "fps_camera.h"
#include "print.h"

#include "../pc/mouse.h"

bool gFPSMode    = TRUE;
bool gNoclipMode = FALSE;

s16 gFpsYaw   = 0;
s16 gFpsPitch = 0;

#define FPS_SENSITIVITY_X 18.0f
#define FPS_SENSITIVITY_Y 18.0f
#define FPS_PITCH_MAX  0x3800
#define FPS_PITCH_MIN -0x3800
#define FPS_EYE_HEIGHT 155.0f

/* -----------------------------------------------------------------------
 * Toggle helpers
 * ----------------------------------------------------------------------- */

void fps_toggle_mode(void) {
    gFPSMode = !gFPSMode;
}

void fps_toggle_noclip(void) {
    gNoclipMode = !gNoclipMode;
}

/* -----------------------------------------------------------------------
 * Stage 1 – integrate mouse, set c->yaw
 * Called from the TOP of execute_mario_action(), BEFORE update_mario_inputs().
 * This ensures c->yaw is current when intendedYaw is computed from the stick.
 *
 * WASD alignment derivation:
 *   update_mario_joystick_inputs computes:
 *       intendedYaw = atan2s(-stickY, stickX) + camera->yaw
 *   Pressing W  →  stickY = 127, stickX = 0
 *       atan2s(-127, 0) = -0x4000  (−90°)
 *   For W to move in the direction the camera faces (gFpsYaw):
 *       intendedYaw = gFpsYaw  ⟹  c->yaw = gFpsYaw + 0x4000
 * ----------------------------------------------------------------------- */
void fps_camera_process_mouse(struct Camera *c) {
    if (!gFPSMode) {
        return;
    }

    int dx = gMouseDeltaX;
    int dy = gMouseDeltaY;
    gMouseDeltaX = 0;
    gMouseDeltaY = 0;

    gFpsYaw -= (s16)(dx * FPS_SENSITIVITY_X);
    gFpsPitch -= (s16)(dy * FPS_SENSITIVITY_Y);
    if (gFpsPitch >  FPS_PITCH_MAX) gFpsPitch =  FPS_PITCH_MAX;
    if (gFpsPitch <  FPS_PITCH_MIN) gFpsPitch =  FPS_PITCH_MIN;

    /* Write the corrected yaw offset so WASD movement is camera-relative.
     *
     * SM64 camera->yaw = yaw of vector FROM focus TO camera (i.e., behind Mario).
     * That is the look-direction + 0x8000 (180°).  We need:
     *   intendedYaw  =  atan2s(-stickY, stickX) + c->yaw
     *   W (stickY=64): atan2s(-64,0) = 0x8000; 0x8000 + c->yaw = gFpsYaw
     *   => c->yaw = gFpsYaw - 0x8000 = gFpsYaw + 0x8000 (s16 wraparound)     */
    c->yaw = (s16)(gFpsYaw + 0x8000);
}

/* -----------------------------------------------------------------------
 * Stage 2 – write eye pos/focus into gLakituState
 * Called at the START of update_camera() (early-exit path when gFPSMode).
 * By this point Mario has already been moved for this frame, so we use his
 * current pos[] directly.
 *
 * Writing both the "cur" fields and the "goal" fields prevents any residual
 * Lakitu smoothing from pulling the camera off the eye position.
 * Writing the "pos"/"focus" fields (0x8C/0x80) is what geo_process_camera
 * actually reads for rendering.
 * ----------------------------------------------------------------------- */
void fps_camera_update(struct Camera *c) {
    if (!gFPSMode || gMarioState == NULL) {
        return;
    }

    c->yaw = (s16)(gFpsYaw + 0x8000);

    Vec3f eyePos;
    eyePos[0] = gMarioState->pos[0];
    eyePos[1] = gMarioState->pos[1] + FPS_EYE_HEIGHT;
    eyePos[2] = gMarioState->pos[2];

    /* Look direction:
     *   forward = ( sins(yaw)*coss(pitch),  sins(pitch),  coss(yaw)*coss(pitch) )
     * sins/coss are SM64 lookup-table macros (s16 angle → float [-1, 1]).
     * Focus is placed 200 SM64 units ahead of the eye. */
    f32 cosP = coss(gFpsPitch);
    Vec3f focusPos;
    focusPos[0] = eyePos[0] + sins(gFpsYaw) * cosP        * 200.0f;
    focusPos[1] = eyePos[1] + sins(gFpsPitch)              * 200.0f;
    focusPos[2] = eyePos[2] + coss(gFpsYaw) * cosP        * 200.0f;

    vec3f_copy(gLakituState.pos,       eyePos);
    vec3f_copy(gLakituState.curPos,    eyePos);
    vec3f_copy(gLakituState.goalPos,   eyePos);
    vec3f_copy(gLakituState.focus,     focusPos);
    vec3f_copy(gLakituState.curFocus,  focusPos);
    vec3f_copy(gLakituState.goalFocus, focusPos);
    gLakituState.yaw     = c->yaw;
    gLakituState.nextYaw = c->yaw;
}

/* -----------------------------------------------------------------------
 * FPS debug HUD — called from render_hud() when gFPSMode is active.
 * Displays horizontal speed in units/second and the player's XYZ position.
 * Text is drawn in the bottom-left corner, away from normal HUD elements.
 * ----------------------------------------------------------------------- */
void fps_draw_hud(void) {
    if (!gFPSMode || gMarioState == NULL) {
        return;
    }

    f32 *vel = gMarioState->vel;
    f32 *pos = gMarioState->pos;
    s32 hspeed = (s32)(sqrtf(vel[0] * vel[0] + vel[2] * vel[2]) * 30.0f);

    print_text_fmt_int(20, 68, "SPD %d", hspeed);
    print_text_fmt_int(20, 52, "X   %d", (s32)pos[0]);
    print_text_fmt_int(20, 36, "Y   %d", (s32)pos[1]);
    print_text_fmt_int(20, 20, "Z   %d", (s32)pos[2]);
}
