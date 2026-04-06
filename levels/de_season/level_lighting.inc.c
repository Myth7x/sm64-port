#include "src/pc/gfx/level_lights.h"
#include "level_table.h"
static void s_lighting_apply(void) {
    level_lights_set_ambient(1.000000f, 1.000000f, 1.000000f, 1.0f);
    level_lights_set_sun(-0.272320f, 0.838671f, -0.471671f, 1.000000f, 0.875502f, 0.582329f);
    level_lights_set_shadow_count(1);
    level_lights_compute_shadow_vp_sun(0, 0.0f, 0.0f, 0.0f, 10000.0f);
    { LevelLight _pl = {{3416.0f, 394.0f, -3248.0f}, 1311.5f, {0.850980f, 1.000000f, 0.952941f}, 4.300000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-296.0f, 264.0f, -3344.0f}, 1311.5f, {0.850980f, 1.000000f, 0.952941f}, 4.300000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-16.0f, 264.0f, -3344.0f}, 1311.5f, {0.850980f, 1.000000f, 0.952941f}, 4.300000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{6640.0f, 40.0f, -208.0f}, 1311.5f, {0.850980f, 1.000000f, 0.952941f}, 4.300000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{6128.0f, 40.0f, -208.0f}, 1311.5f, {0.850980f, 1.000000f, 0.952941f}, 4.300000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{5488.0f, 80.0f, -504.0f}, 1311.5f, {0.850980f, 1.000000f, 0.952941f}, 4.300000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{5488.0f, 80.0f, -272.0f}, 1311.5f, {0.850980f, 1.000000f, 0.952941f}, 4.300000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-1292.0f, 408.0f, -2968.0f}, 1311.5f, {0.850980f, 1.000000f, 0.952941f}, 4.300000f}; level_lights_add(&_pl); }
    level_lights_set_fog(0.392157f, 0.470588f, 0.509804f, 256.0f, 5500.0f, 1.0000f);
}
__attribute__((constructor)) static void s_lighting_register(void) {
    level_lighting_register(LEVEL_LIGHTING_NUM, s_lighting_apply);
}
