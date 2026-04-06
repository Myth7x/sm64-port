#include "src/pc/gfx/level_lights.h"
#include "level_table.h"
static void s_lighting_apply(void) {
    level_lights_set_ambient(0.247059f, 0.334118f, 0.388235f, 1.0f);
    level_lights_set_sun(0.253132f, 0.573576f, 0.779060f, 1.000000f, 0.659574f, 0.302128f);
    level_lights_set_shadow_count(1);
    level_lights_compute_shadow_vp_sun(0, 0.0f, 0.0f, 0.0f, 10000.0f);
}
__attribute__((constructor)) static void s_lighting_register(void) {
    level_lighting_register(LEVEL_LIGHTING_NUM, s_lighting_apply);
}
