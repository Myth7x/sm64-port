#include "sm64.h"
#include "entity_boxes.h"
#include "fps_mode.h"

static const struct EntityBox *sEntityBoxes = NULL;
static f32 sCheckpoint[3] = {0.0f, 0.0f, 0.0f};

void entity_boxes_register(const struct EntityBox *boxes) {
    sEntityBoxes = boxes;
}

const struct EntityBox *entity_boxes_get(void) {
    return sEntityBoxes;
}

void entity_boxes_clear(void) {
    sEntityBoxes = NULL;
}

void entity_boxes_set_default_spawn(f32 x, f32 y, f32 z) {
    sCheckpoint[0] = x;
    sCheckpoint[1] = y;
    sCheckpoint[2] = z;
}

void entity_boxes_update(struct MarioState *m) {
    if (sEntityBoxes == NULL || m->marioObj == NULL || gNoclipMode) return;
    static s32 sTeleportCooldown = 0;
    static s32 sDeathCooldown = 0;
    static s32 sPushCooldown = 0;
    if (sTeleportCooldown > 0) sTeleportCooldown--;
    if (sDeathCooldown > 0) sDeathCooldown--;
    if (sPushCooldown > 0) sPushCooldown--;
    f32 px = m->pos[0], py = m->pos[1], pz = m->pos[2];
    for (const struct EntityBox *b = sEntityBoxes; b->type != 0; b++) {
        if (px < b->mins[0] || px > b->maxs[0]) continue;
        if (py + 160.0f < b->mins[1] || py > b->maxs[1]) continue;
        if (pz < b->mins[2] || pz > b->maxs[2]) continue;
        switch (b->type) {
            case EBOX_TELEPORT:
                if (sTeleportCooldown > 0) break;
                m->pos[0] = (f32)b->dest[0];
                m->pos[1] = (f32)b->dest[1];
                m->pos[2] = (f32)b->dest[2];
                m->marioObj->oPosX = m->pos[0];
                m->marioObj->oPosY = m->pos[1];
                m->marioObj->oPosZ = m->pos[2];
                m->vel[0] = 0.0f; m->vel[1] = 0.0f; m->vel[2] = 0.0f;
                m->forwardVel = 0.0f;
                sTeleportCooldown = 30;
                break;
            case EBOX_DEATH:
                if (sDeathCooldown > 0) break;
                m->pos[0] = sCheckpoint[0];
                m->pos[1] = sCheckpoint[1];
                m->pos[2] = sCheckpoint[2];
                m->marioObj->oPosX = m->pos[0];
                m->marioObj->oPosY = m->pos[1];
                m->marioObj->oPosZ = m->pos[2];
                m->vel[0] = 0.0f; m->vel[1] = 0.0f; m->vel[2] = 0.0f;
                m->forwardVel = 0.0f;
                sDeathCooldown = 30;
                break;
            case EBOX_SCRIPT:
                sCheckpoint[0] = px;
                sCheckpoint[1] = py;
                sCheckpoint[2] = pz;
                break;
            case EBOX_PUSH:
                if (sPushCooldown > 0) break;
                m->vel[0] += (f32)b->dest[0];
                m->vel[1] += (f32)b->dest[1];
                m->vel[2] += (f32)b->dest[2];
                sPushCooldown = 10;
                break;
        }
    }
}
