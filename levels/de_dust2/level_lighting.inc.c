#include "src/pc/gfx/level_lights.h"
#include "level_table.h"
static void s_lighting_apply(void) {
    level_lights_set_ambient(1.000000f, 0.946058f, 0.788382f, 1.0f);
    level_lights_set_sun(0.365677f, 0.866025f, -0.340999f, 1.000000f, 0.965665f, 0.862661f);
    level_lights_set_shadow_count(1);
    level_lights_compute_shadow_vp_sun(0, 0.0f, 0.0f, 0.0f, 10000.0f);
    { LevelLight _pl = {{-2812.0f, 528.0f, -2304.0f}, 547.7f, {0.996078f, 0.984314f, 0.752941f}, 0.750000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-2300.0f, 528.0f, -2304.0f}, 547.7f, {0.996078f, 0.984314f, 0.752941f}, 0.750000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-3324.0f, 528.0f, -2304.0f}, 547.7f, {0.996078f, 0.984314f, 0.752941f}, 0.750000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-3836.0f, 528.0f, -2304.0f}, 547.7f, {0.996078f, 0.984314f, 0.752941f}, 0.750000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-1996.0f, 112.0f, -2848.0f}, 547.7f, {0.996078f, 0.984314f, 0.752941f}, 0.750000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-1484.0f, 112.0f, -2848.0f}, 547.7f, {0.996078f, 0.984314f, 0.752941f}, 0.750000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-2814.0f, 620.0f, -2304.0f}, 316.2f, {1.000000f, 1.000000f, 1.000000f}, 0.250000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-2816.0f, 248.0f, -2304.0f}, 316.2f, {0.996078f, 0.984314f, 0.752941f}, 0.250000f}; level_lights_add(&_pl); }
    level_lights_set_fog(0.772549f, 0.768627f, 0.647059f, 500.0f, 4000.0f, 1.0000f);
}
__attribute__((constructor)) static void s_lighting_register(void) {
    level_lighting_register(LEVEL_LIGHTING_NUM, s_lighting_apply);
}
