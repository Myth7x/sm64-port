#include "src/pc/gfx/level_lights.h"
#include "level_table.h"
static void s_lighting_apply(void) {
    level_lights_set_ambient(0.807018f, 1.000000f, 0.719298f, 1.0f);
    level_lights_set_sun(0.178606f, 0.906308f, 0.383022f, 0.741176f, 0.847059f, 0.662745f);
    level_lights_set_shadow_count(1);
    level_lights_compute_shadow_vp_sun(0, 0.0f, 0.0f, 0.0f, 10000.0f);
    { LevelLight _pl = {{-17408.0f, -11264.0f, 17920.0f}, 894.4f, {0.686275f, 0.454902f, 0.149020f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-16384.0f, -10240.0f, 17920.0f}, 894.4f, {0.686275f, 0.454902f, 0.149020f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-16896.0f, -10752.0f, 17920.0f}, 894.4f, {0.686275f, 0.454902f, 0.149020f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-15872.0f, -9728.0f, 17920.0f}, 894.4f, {0.686275f, 0.454902f, 0.149020f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-15360.0f, -9216.0f, 17920.0f}, 894.4f, {0.686275f, 0.454902f, 0.149020f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-17408.0f, -11264.0f, 18944.0f}, 894.4f, {0.686275f, 0.454902f, 0.149020f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-16384.0f, -10240.0f, 18944.0f}, 894.4f, {0.686275f, 0.454902f, 0.149020f}, 10.000000f}; level_lights_add(&_pl); }
    { LevelLight _pl = {{-16896.0f, -10752.0f, 18944.0f}, 894.4f, {0.686275f, 0.454902f, 0.149020f}, 10.000000f}; level_lights_add(&_pl); }
    level_lights_set_fog(0.729412f, 0.756863f, 0.796078f, 128.0f, 3000.0f, 0.8000f);
}
__attribute__((constructor)) static void s_lighting_register(void) {
    level_lighting_register(LEVEL_LIGHTING_NUM, s_lighting_apply);
}
