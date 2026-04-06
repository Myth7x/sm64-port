#include "sm64.h"
#include "game/object_helpers.h"
#include "game/object_list_processor.h"
#include "game/level_update.h"
#include "engine/surface_load.h"
#include "bhv_moving_platform.h"

#define o gCurrentObject

static const struct LevelMovingPlatform *s_platforms = NULL;

void moving_platform_register(const struct LevelMovingPlatform *plats) {
    s_platforms = plats;
}

void bhv_moving_platform_loop(void) {
    if (s_platforms == NULL) return;
    s32 idx = o->oBehParams2ndByte;
    const struct LevelMovingPlatform *p = &s_platforms[idx];
    if (o->oTimer == 0) {
        obj_set_collision_data(o, p->collision);
        o->oCollisionDistance = 3000;
        o->oMoveAnglePitch = 0;
        o->oSubAction = 0;
    }
    if (p->mode == PLAT_MODE_AUTO) {
        if (p->period > 0) {
            s32 t = o->oTimer % (p->period * 2);
            f32 frac = (t < p->period) ? ((f32)t / p->period) : ((f32)(p->period * 2 - t) / p->period);
            o->oPosX = (f32)p->start[0] + frac * (f32)(p->end[0] - p->start[0]);
            o->oPosY = (f32)p->start[1] + frac * (f32)(p->end[1] - p->start[1]);
            o->oPosZ = (f32)p->start[2] + frac * (f32)(p->end[2] - p->start[2]);
        }
    } else {
        struct MarioState *m = &gMarioStates[0];
        s32 in_act;
        if (p->mode == PLAT_MODE_STEP) {
            in_act = (m->floor != NULL && m->floor->object == o);
        } else {
            f32 px = m->pos[0], py = m->pos[1], pz = m->pos[2];
            in_act = (px >= (f32)p->activator_mins[0] && px <= (f32)p->activator_maxs[0] &&
                          py + 160.0f >= (f32)p->activator_mins[1] && py <= (f32)p->activator_maxs[1] &&
                          pz >= (f32)p->activator_mins[2] && pz <= (f32)p->activator_maxs[2]);
            if (p->mode == PLAT_MODE_USE)
                in_act = in_act && (m->controller->buttonPressed & B_BUTTON);
        }
        switch (o->oSubAction) {
            case 0:
                o->oPosX = (f32)p->start[0];
                o->oPosY = (f32)p->start[1];
                o->oPosZ = (f32)p->start[2];
                if (in_act) { o->oSubAction = 1; o->oMoveAnglePitch = 0; }
                break;
            case 1:
                o->oMoveAnglePitch++;
                if (p->period > 0) {
                    f32 frac = (f32)o->oMoveAnglePitch / (f32)p->period;
                    if (frac >= 1.0f) { frac = 1.0f; o->oSubAction = 2; o->oMoveAnglePitch = 0; }
                    o->oPosX = (f32)p->start[0] + frac * (f32)(p->end[0] - p->start[0]);
                    o->oPosY = (f32)p->start[1] + frac * (f32)(p->end[1] - p->start[1]);
                    o->oPosZ = (f32)p->start[2] + frac * (f32)(p->end[2] - p->start[2]);
                } else {
                    o->oSubAction = 2; o->oMoveAnglePitch = 0;
                }
                break;
            case 2:
                o->oPosX = (f32)p->end[0];
                o->oPosY = (f32)p->end[1];
                o->oPosZ = (f32)p->end[2];
                if (p->wait_frames == -1) break;
                o->oMoveAnglePitch++;
                if (o->oMoveAnglePitch >= p->wait_frames) { o->oSubAction = 3; o->oMoveAnglePitch = 0; }
                break;
            case 3:
                o->oMoveAnglePitch++;
                if (p->period > 0) {
                    f32 frac = (f32)o->oMoveAnglePitch / (f32)p->period;
                    if (frac >= 1.0f) { frac = 1.0f; o->oSubAction = 0; o->oMoveAnglePitch = 0; }
                    o->oPosX = (f32)p->end[0] + frac * (f32)(p->start[0] - p->end[0]);
                    o->oPosY = (f32)p->end[1] + frac * (f32)(p->start[1] - p->end[1]);
                    o->oPosZ = (f32)p->end[2] + frac * (f32)(p->start[2] - p->end[2]);
                } else {
                    o->oSubAction = 0; o->oMoveAnglePitch = 0;
                }
                break;
        }
    }
    load_object_collision_model();
}
