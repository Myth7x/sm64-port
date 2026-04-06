#include "src/pc/gfx/level_lights.h"
#include "level_table.h"
static void s_lighting_apply(void) {
    level_lights_set_ambient(0.200000f, 0.217647f, 0.233333f, 1.0f);
    level_lights_set_sun(-0.357749f, 0.891007f, 0.279504f, 1.000000f, 0.933333f, 0.776471f);
    level_lights_set_shadow_count(1);
    level_lights_compute_shadow_vp_sun(0, 0.0f, 0.0f, 0.0f, 10000.0f);
    { LevelLight _pl = {{-3110.0f, -13055.0f, -25822.0f}, 894.4f, {1.000000f, 0.937255f, 0.749020f}, 2.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-18150.0f, 3716.0f, -976.0f}, 894.4f, {1.000000f, 0.956863f, 0.807843f}, 2.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-11870.0f, 3804.0f, -3252.0f}, 894.4f, {1.000000f, 0.956863f, 0.807843f}, 2.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-11736.0f, 3590.0f, -11624.0f}, 894.4f, {1.000000f, 0.956863f, 0.807843f}, 2.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-17924.0f, 3658.0f, -17328.0f}, 894.4f, {1.000000f, 0.956863f, 0.807843f}, 2.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-4590.0f, -12992.0f, -26882.0f}, 894.4f, {1.000000f, 0.949020f, 0.800000f}, 2.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-3691.0f, -12896.0f, -27362.0f}, 894.4f, {1.000000f, 0.937255f, 0.749020f}, 2.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-4398.0f, -12896.0f, -25762.0f}, 894.4f, {1.000000f, 0.937255f, 0.749020f}, 2.000000f}; level_lights_add(&_pl); }
    level_lights_set_fog(0.745098f, 0.745098f, 0.745098f, 2800.0f, 6000.0f, 1.0000f);
}
__attribute__((constructor)) static void s_lighting_register(void) {
    level_lighting_register(LEVEL_LIGHTING_NUM, s_lighting_apply);
}
