#include "src/pc/gfx/level_lights.h"
#include "level_table.h"
static void s_lighting_apply(void) {
    level_lights_set_ambient(0.610837f, 0.679803f, 1.000000f, 1.0f);
    level_lights_set_sun(-0.253132f, 0.951057f, -0.177245f, 1.000000f, 0.944681f, 0.753191f);
    level_lights_set_shadow_count(1);
    level_lights_compute_shadow_vp_sun(0, 0.0f, 0.0f, 0.0f, 10000.0f);
    { LevelLight _pl = {{15264.0f, 9216.0f, -17376.0f}, 2000.0f, {1.000000f, 1.000000f, 1.000000f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{13280.0f, 9216.0f, -17374.0f}, 2000.0f, {1.000000f, 1.000000f, 1.000000f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{13280.0f, 9216.0f, -15392.0f}, 2000.0f, {1.000000f, 1.000000f, 1.000000f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{15264.0f, 9216.0f, -15394.0f}, 2000.0f, {1.000000f, 1.000000f, 1.000000f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{16736.0f, 7424.0f, -17374.0f}, 2000.0f, {1.000000f, 1.000000f, 1.000000f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{14752.0f, 7424.0f, -17373.0f}, 2000.0f, {1.000000f, 1.000000f, 1.000000f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{14752.0f, 7424.0f, -15390.0f}, 2000.0f, {1.000000f, 1.000000f, 1.000000f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{16736.0f, 7424.0f, -15392.0f}, 2000.0f, {1.000000f, 1.000000f, 1.000000f}, 10.000000f}; level_lights_add(&_pl); }
}
__attribute__((constructor)) static void s_lighting_register(void) {
    level_lighting_register(LEVEL_LIGHTING_NUM, s_lighting_apply);
}
