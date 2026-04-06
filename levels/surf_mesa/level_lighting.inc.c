#include "src/pc/gfx/level_lights.h"
#include "level_table.h"
static void s_lighting_apply(void) {
    level_lights_set_ambient(0.726744f, 0.843023f, 1.000000f, 1.0f);
    level_lights_set_sun(-0.000000f, 0.469472f, 0.882948f, 1.000000f, 0.944681f, 0.753191f);
    level_lights_set_shadow_count(1);
    level_lights_compute_shadow_vp_sun(0, 0.0f, 0.0f, 0.0f, 10000.0f);
    level_lights_set_fog(0.611765f, 0.925490f, 0.980392f, 5000.0f, 20000.0f, 1.0000f);
}
__attribute__((constructor)) static void s_lighting_register(void) {
    level_lighting_register(LEVEL_LIGHTING_NUM, s_lighting_apply);
}
