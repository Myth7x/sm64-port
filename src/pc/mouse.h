#ifndef MOUSE_H
#define MOUSE_H

/**
 * Raw per-frame mouse motion deltas, written by the SDL window backend
 * (gfx_sdl2.c) and consumed + cleared by fps_camera_update() each frame.
 */
extern int gMouseDeltaX;
extern int gMouseDeltaY;

/**
 * Accumulate relative mouse motion. Called from the SDL event loop for every
 * SDL_MOUSEMOTION event. Values are summed until fps_camera_update() resets
 * them to 0.
 */
void mouse_accum(int dx, int dy);

#endif
