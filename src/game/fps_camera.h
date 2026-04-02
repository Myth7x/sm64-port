#ifndef FPS_CAMERA_H
#define FPS_CAMERA_H

#include "types.h"
#include "camera.h"

/**
 * Current FPS camera angles (SM64 s16 angle units, 0x10000 = 360 degrees).
 * Exposed so that source_movement.c can read yaw/pitch for noclip movement.
 */
extern s16 gFpsYaw;
extern s16 gFpsPitch;

/**
 * Stage 1 (called from the TOP of execute_mario_action, before update_mario_inputs).
 *
 * Reads and clears accumulated mouse deltas, then updates gFpsYaw/gFpsPitch and
 * writes c->yaw = gFpsYaw + 0x4000 so that WASD movement is camera-relative.
 * The +0x4000 compensation comes from atan2s(-stickY, stickX) returning -0x4000
 * for a forward (W) key press, requiring camera->yaw = gFpsYaw + 0x4000 for the
 * resulting intendedYaw to equal gFpsYaw.
 */
void fps_camera_process_mouse(struct Camera *c);

/**
 * Stage 2 (called at the start of update_camera, after Mario has moved).
 *
 * Uses Mario's current world position (updated by source_movement this frame)
 * and the current gFpsYaw/gFpsPitch to compute the eye position and focus point,
 * then writes them directly into gLakituState, bypassing Lakitu smoothing.
 * Returns immediately so the rest of update_camera is skipped.
 */
void fps_camera_update(struct Camera *c);

/**
 * Toggle gFPSMode on/off.  Called from the SDL key handler (F key).
 */
void fps_toggle_mode(void);

/**
 * Toggle gNoclipMode on/off.  Called from the SDL key handler (V key).
 */
void fps_toggle_noclip(void);

/**
 * Draw the FPS debug HUD overlay (speed + XYZ) via print_text_fmt_int.
 * Must be called from within render_hud() so that render_text_labels()
 * flushes the queued glyphs in the same frame.
 */
void fps_draw_hud(void);

#endif
