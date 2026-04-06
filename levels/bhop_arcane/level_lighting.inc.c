#include "src/pc/gfx/level_lights.h"
#include "level_table.h"
static void s_lighting_apply(void) {
    level_lights_set_ambient(0.668224f, 0.822430f, 1.000000f, 1.0f);
    level_lights_set_sun(0.489074f, 0.207912f, 0.847101f, 1.000000f, 0.856574f, 0.557769f);
    level_lights_set_shadow_count(1);
    level_lights_compute_shadow_vp_sun(0, 0.0f, 0.0f, 0.0f, 5000.0f);
    level_lights_set_fog(0.627451f, 0.666667f, 0.686275f, 1000.0f, 12000.0f, 1.0000f);
}
__attribute__((constructor)) static void s_lighting_register(void) {
    level_lighting_register(LEVEL_LIGHTING_NUM, s_lighting_apply);
}
