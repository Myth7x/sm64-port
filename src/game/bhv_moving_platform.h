#pragma once

#include "types.h"

#define PLAT_MODE_AUTO  0
#define PLAT_MODE_TOUCH 1
#define PLAT_MODE_USE   2
#define PLAT_MODE_STEP  3

struct LevelMovingPlatform {
    s16 start[3];
    s16 end[3];
    s16 period;
    s16 wait_frames;
    u8  mode;
    u8  _pad[3];
    s16 activator_mins[3];
    s16 activator_maxs[3];
    const Collision *collision;
};

void moving_platform_register(const struct LevelMovingPlatform *plats);
void bhv_moving_platform_loop(void);
