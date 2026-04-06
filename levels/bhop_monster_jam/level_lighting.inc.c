#include "src/pc/gfx/level_lights.h"
#include "level_table.h"
static void s_lighting_apply(void) {
    level_lights_set_ambient(1.000000f, 0.946058f, 0.788382f, 1.0f);
    level_lights_set_sun(0.365677f, 0.866025f, -0.340999f, 0.464706f, 0.454902f, 0.423529f);
    level_lights_set_shadow_count(1);
    level_lights_compute_shadow_vp_sun(0, 0.0f, 0.0f, 0.0f, 10000.0f);
    { LevelLight _pl = {{8370.0f, 8353.0f, -4929.0f}, 1183.2f, {1.000000f, 1.000000f, 1.000000f}, 3.500000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{8366.0f, 8348.0f, -2951.0f}, 1183.2f, {1.000000f, 1.000000f, 1.000000f}, 3.500000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{8374.0f, 8348.0f, -1391.0f}, 1183.2f, {1.000000f, 1.000000f, 1.000000f}, 3.500000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{8768.0f, 8320.0f, -6645.0f}, 1183.2f, {1.000000f, 1.000000f, 1.000000f}, 3.500000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{10080.0f, 8337.0f, -5568.0f}, 1183.2f, {1.000000f, 1.000000f, 1.000000f}, 3.500000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{10496.0f, 12376.0f, -4352.0f}, 894.4f, {1.000000f, 1.000000f, 1.000000f}, 2.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{10240.0f, 8448.0f, -2816.0f}, 774.6f, {1.000000f, 1.000000f, 1.000000f}, 1.500000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{1998.0f, 9856.0f, -768.0f}, 632.5f, {1.000000f, 1.000000f, 1.000000f}, 1.000000f}; level_lights_add(&_pl); }
}
__attribute__((constructor)) static void s_lighting_register(void) {
    level_lighting_register(LEVEL_LIGHTING_NUM, s_lighting_apply);
}
