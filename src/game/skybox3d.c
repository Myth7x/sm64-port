#include <PR/ultratypes.h>
#include <PR/gbi.h>
#include <stdio.h>
#include <stdarg.h>

#include "engine/math_util.h"
#include "game_init.h"
#include "memory.h"
#include "skybox3d.h"

static bool   sSkyActive   = false;

void sky_log(const char *fmt, ...) {
    static int s_opened = 0;
    FILE *f = fopen("sky_debug.log", s_opened ? "a" : "w");
    if (!f) return;
    s_opened = 1;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fclose(f);
}
static Gfx   *sSkyDL       = NULL;
static s32    sSkyOriginX  = 0;
static s32    sSkyOriginY  = 0;
static s32    sSkyOriginZ  = 0;
static s32    sSkyScale    = 16;

void sky3d_register(Gfx *dl, s32 orig_x, s32 orig_y, s32 orig_z, s32 scale) {
    sSkyDL      = dl;
    sSkyOriginX = orig_x;
    sSkyOriginY = orig_y;
    sSkyOriginZ = orig_z;
    sSkyScale   = (scale > 0) ? scale : 16;
    sSkyActive  = true;
    sky_log("[sky3d] register: dl=%p origin=(%d,%d,%d) scale=%d\n",
            (void*)dl, orig_x, orig_y, orig_z, sSkyScale);
}

void sky3d_clear(void) {
    sky_log("[sky3d] clear (was active=%d)\n", (int)sSkyActive);
    sSkyActive = false;
    sSkyDL     = NULL;
}

bool sky3d_is_active(void) {
    return sSkyActive;
}

/**
 * Emit the sky view matrix and sky display list into gDisplayListHead.
 * Called from geo_process_camera() each frame when a sky is registered.
 *
 * The sky camera position is derived from the player camera position:
 *   sky_pos = sky_origin + player_pos / sky_scale
 * This gives correct parallax: the sky appears to move with the player
 * but at 1/sky_scale speed, matching Source Engine's sky_camera behaviour.
 */
void sky3d_geo_inject(Vec3f pos, Vec3f focus, s16 roll) {
    if (!sSkyActive || sSkyDL == NULL) {
        sky_log("[sky3d] geo_inject skipped: active=%d dl=%p\n",
                (int)sSkyActive, (void*)sSkyDL);
        return;
    }

    Vec3f sky_pos, sky_focus;
    sky_pos[0]   = (f32)sSkyOriginX + pos[0]   / (f32)sSkyScale;
    sky_pos[1]   = (f32)sSkyOriginY + pos[1]   / (f32)sSkyScale;
    sky_pos[2]   = (f32)sSkyOriginZ + pos[2]   / (f32)sSkyScale;
    sky_focus[0] = (f32)sSkyOriginX + focus[0] / (f32)sSkyScale;
    sky_focus[1] = (f32)sSkyOriginY + focus[1] / (f32)sSkyScale;
    sky_focus[2] = (f32)sSkyOriginZ + focus[2] / (f32)sSkyScale;

    /* Log first few frames so we can verify camera position vs geometry */
    static int s_inject_count = 0;
    if (s_inject_count < 5) {
        sky_log("[sky3d] inject #%d: player_pos=(%.0f,%.0f,%.0f)"
                " sky_pos=(%.0f,%.0f,%.0f)"
                " sky_focus=(%.0f,%.0f,%.0f)"
                " origin=(%d,%d,%d) scale=%d dl=%p\n",
                s_inject_count,
                pos[0], pos[1], pos[2],
                sky_pos[0], sky_pos[1], sky_pos[2],
                sky_focus[0], sky_focus[1], sky_focus[2],
                sSkyOriginX, sSkyOriginY, sSkyOriginZ, sSkyScale,
                (void*)sSkyDL);
        /* Log first word of DL so we can confirm it's a valid GBI command */
        sky_log("[sky3d]   dl[0]=0x%08X dl[1]=0x%08X\n",
                (unsigned)sSkyDL[0].words.w0, (unsigned)sSkyDL[0].words.w1);
    }
    s_inject_count++;

    Mat4 sky_mv;
    mtxf_lookat(sky_mv, sky_pos, sky_focus, roll);

    Mtx *sky_mtx = alloc_display_list(sizeof(Mtx));
    if (sky_mtx == NULL) {
        sky_log("[sky3d] ERROR: alloc_display_list failed for sky matrix!\n");
        return;
    }
    mtxf_to_mtx(sky_mtx, sky_mv);

    gSPMatrix(gDisplayListHead++,
              VIRTUAL_TO_PHYSICAL(sky_mtx),
              G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPDisplayList(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(sSkyDL));
}
