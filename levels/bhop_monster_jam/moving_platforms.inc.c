#include <ultra64.h>
#include "sm64.h"
#include "game/bhv_moving_platform.h"

#include "levels/bhop_monster_jam/moving_0.inc.c"
#include "levels/bhop_monster_jam/moving_0_col.inc.c"

#include "levels/bhop_monster_jam/moving_1.inc.c"
#include "levels/bhop_monster_jam/moving_1_col.inc.c"

#include "levels/bhop_monster_jam/moving_2.inc.c"
#include "levels/bhop_monster_jam/moving_2_col.inc.c"

#include "levels/bhop_monster_jam/moving_3.inc.c"
#include "levels/bhop_monster_jam/moving_3_col.inc.c"

#include "levels/bhop_monster_jam/moving_4.inc.c"
#include "levels/bhop_monster_jam/moving_4_col.inc.c"

#include "levels/bhop_monster_jam/moving_5.inc.c"
#include "levels/bhop_monster_jam/moving_5_col.inc.c"

#include "levels/bhop_monster_jam/moving_6.inc.c"
#include "levels/bhop_monster_jam/moving_6_col.inc.c"

#include "levels/bhop_monster_jam/moving_7.inc.c"
#include "levels/bhop_monster_jam/moving_7_col.inc.c"

#include "levels/bhop_monster_jam/moving_8.inc.c"
#include "levels/bhop_monster_jam/moving_8_col.inc.c"

#include "levels/bhop_monster_jam/moving_9.inc.c"
#include "levels/bhop_monster_jam/moving_9_col.inc.c"

#include "levels/bhop_monster_jam/moving_10.inc.c"
#include "levels/bhop_monster_jam/moving_10_col.inc.c"

#include "levels/bhop_monster_jam/moving_11.inc.c"
#include "levels/bhop_monster_jam/moving_11_col.inc.c"

#include "levels/bhop_monster_jam/moving_12.inc.c"
#include "levels/bhop_monster_jam/moving_12_col.inc.c"

#include "levels/bhop_monster_jam/moving_13.inc.c"
#include "levels/bhop_monster_jam/moving_13_col.inc.c"

#include "levels/bhop_monster_jam/moving_14.inc.c"
#include "levels/bhop_monster_jam/moving_14_col.inc.c"

#include "levels/bhop_monster_jam/moving_15.inc.c"
#include "levels/bhop_monster_jam/moving_15_col.inc.c"

#include "levels/bhop_monster_jam/moving_16.inc.c"
#include "levels/bhop_monster_jam/moving_16_col.inc.c"

#include "levels/bhop_monster_jam/moving_17.inc.c"
#include "levels/bhop_monster_jam/moving_17_col.inc.c"

#include "levels/bhop_monster_jam/moving_18.inc.c"
#include "levels/bhop_monster_jam/moving_18_col.inc.c"

#include "levels/bhop_monster_jam/moving_19.inc.c"
#include "levels/bhop_monster_jam/moving_19_col.inc.c"

#include "levels/bhop_monster_jam/moving_20.inc.c"
#include "levels/bhop_monster_jam/moving_20_col.inc.c"

#include "levels/bhop_monster_jam/moving_21.inc.c"
#include "levels/bhop_monster_jam/moving_21_col.inc.c"

#include "levels/bhop_monster_jam/moving_22.inc.c"
#include "levels/bhop_monster_jam/moving_22_col.inc.c"

#include "levels/bhop_monster_jam/moving_23.inc.c"
#include "levels/bhop_monster_jam/moving_23_col.inc.c"

#include "levels/bhop_monster_jam/moving_24.inc.c"
#include "levels/bhop_monster_jam/moving_24_col.inc.c"

#include "levels/bhop_monster_jam/moving_25.inc.c"
#include "levels/bhop_monster_jam/moving_25_col.inc.c"

#include "levels/bhop_monster_jam/moving_26.inc.c"
#include "levels/bhop_monster_jam/moving_26_col.inc.c"

#include "levels/bhop_monster_jam/moving_27.inc.c"
#include "levels/bhop_monster_jam/moving_27_col.inc.c"

#include "levels/bhop_monster_jam/moving_28.inc.c"
#include "levels/bhop_monster_jam/moving_28_col.inc.c"

#include "levels/bhop_monster_jam/moving_29.inc.c"
#include "levels/bhop_monster_jam/moving_29_col.inc.c"

static const struct LevelMovingPlatform bhop_monster_jam_moving_platforms[] = {
    {{1207, 7936, -3529}, {1207, 8944, -3529}, 504, -1, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_0_area_1_collision},
    {{3520, 9024, -3504}, {4016, 9024, -3504}, 74, 120, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_1_area_1_collision},
    {{3296, 8928, -3472}, {3472, 8928, -3472}, 26, 120, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_2_area_1_collision},
    {{10368, 8328, -2170}, {10368, 8328, -2170}, 1, 90, 0, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_3_area_1_collision},
    {{10368, 8328, -3466}, {10368, 8328, -3466}, 1, 90, 0, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_4_area_1_collision},
    {{1392, 7552, 5648}, {1392, 7552, 5648}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_5_area_1_collision},
    {{1392, 7552, 6032}, {1392, 7552, 6032}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_6_area_1_collision},
    {{1264, 7552, 6224}, {1264, 7552, 6224}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_7_area_1_collision},
    {{2272, 7552, 6656}, {2272, 7552, 6656}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_8_area_1_collision},
    {{2448, 7552, 6896}, {2448, 7552, 6896}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_9_area_1_collision},
    {{2672, 7552, 6672}, {2672, 7552, 6672}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_10_area_1_collision},
    {{2896, 7552, 6960}, {2896, 7552, 6960}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_11_area_1_collision},
    {{3152, 7552, 6832}, {3152, 7552, 6832}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_12_area_1_collision},
    {{3696, 7552, 6768}, {3696, 7552, 6768}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_13_area_1_collision},
    {{3424, 7552, 6656}, {3424, 7552, 6656}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_14_area_1_collision},
    {{3472, 7552, 6064}, {3472, 7552, 6064}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_15_area_1_collision},
    {{3888, 7552, 6064}, {3888, 7552, 6064}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_16_area_1_collision},
    {{3664, 7552, 5936}, {3664, 7552, 5936}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_17_area_1_collision},
    {{3664, 7552, 5648}, {3664, 7552, 5648}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_18_area_1_collision},
    {{3856, 7552, 5360}, {3856, 7552, 5360}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_19_area_1_collision},
    {{4112, 7552, 5264}, {4112, 7552, 5264}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_20_area_1_collision},
    {{4272, 7552, 4976}, {4272, 7552, 4976}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_21_area_1_collision},
    {{4272, 7552, 4720}, {4272, 7552, 4720}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_22_area_1_collision},
    {{3984, 7552, 4464}, {3984, 7552, 4464}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_23_area_1_collision},
    {{3856, 7552, 4208}, {3856, 7552, 4208}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_24_area_1_collision},
    {{3952, 7552, 3888}, {3952, 7552, 3888}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_25_area_1_collision},
    {{9360, 7552, 3088}, {9360, 7552, 3088}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_26_area_1_collision},
    {{9360, 7552, 3344}, {9360, 7552, 3344}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_27_area_1_collision},
    {{9232, 7552, 3568}, {9232, 7552, 3568}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_28_area_1_collision},
    {{9360, 7552, 3792}, {9360, 7552, 3792}, 1, 9, 3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, bhop_monster_jam_moving_29_area_1_collision},
};

__attribute__((constructor)) static void s_bhop_monster_jam_register_platforms(void) {
    moving_platform_register(bhop_monster_jam_moving_platforms);
}
