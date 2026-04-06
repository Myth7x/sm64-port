#include "src/pc/gfx/level_lights.h"
#include "level_table.h"
static void s_lighting_apply(void) {
    level_lights_set_ambient(0.266667f, 0.347059f, 0.411765f, 1.0f);
    level_lights_set_sun(0.449397f, 0.866025f, -0.219186f, 1.000000f, 0.912698f, 0.781746f);
    level_lights_set_shadow_count(1);
    level_lights_compute_shadow_vp_sun(0, 0.0f, 0.0f, 0.0f, 10000.0f);
    level_lights_set_fog(0.619608f, 0.654902f, 0.650980f, 512.0f, 11500.0f, 1.0000f);
}
__attribute__((constructor)) static void s_lighting_register(void) {
    level_lighting_register(LEVEL_LIGHTING_NUM, s_lighting_apply);
}
