#pragma once

#include "types.h"

#define EBOX_DEATH    1
#define EBOX_TELEPORT 2
#define EBOX_SCRIPT   3
#define EBOX_DOOR     4
#define EBOX_BRUSH    5
#define EBOX_LOGIC    6
#define EBOX_LANDMARK 7
#define EBOX_PUSH     8

struct EntityBox {
    s16 type;
    s16 mins[3];
    s16 maxs[3];
    s16 dest[3];
};

void entity_boxes_register(const struct EntityBox *boxes);
const struct EntityBox *entity_boxes_get(void);
void entity_boxes_clear(void);
void entity_boxes_update(struct MarioState *m);
void entity_boxes_set_default_spawn(f32 x, f32 y, f32 z);
