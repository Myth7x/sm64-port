#pragma once

#include <stdint.h>

#define LEVEL_LIGHTS_MAX  8
#define LEVEL_SHADOW_MAX  4
#define SHADOW_MAP_SIZE   1024

typedef struct {
    float position[3];
    float radius;
    float color[3];
    float intensity;
} LevelLight;

typedef struct {
    float ambient_color[4];
    float light_pos[LEVEL_LIGHTS_MAX][4];
    float light_color[LEVEL_LIGHTS_MAX][4];
    float shadow_vp[LEVEL_SHADOW_MAX][16];
    int   shadow_count;
    float _pad[3];
    float sun_dir[4];
    float sun_color[4];
} LightsCBData;

typedef struct {
    float fog_color[4];
    float fog_near;
    float fog_far;
    float fog_max_density;
    float fog_enabled;
    float _fog_pad[56];
} FogCBData;

#ifdef __cplusplus
extern "C" {
#endif

void level_lights_clear(void);
void level_lights_set_ambient(float r, float g, float b, float intensity);
void level_lights_add(const LevelLight *light);
int  level_lights_count(void);
void level_lights_get(int idx, LevelLight *out);
void level_lights_set(int idx, const LevelLight *light);
void level_lights_fill_cb(LightsCBData *out);

void level_lights_set_shadow_count(int n);
void level_lights_set_shadow_vp(int idx, const float mat4x4[16]);
void level_lights_compute_shadow_vp(int idx, int light_idx,
                                    float scene_cx, float scene_cy, float scene_cz,
                                    float scene_radius);
void level_lights_set_sun(float dx, float dy, float dz, float r, float g, float b);

typedef void (*LevelLightingFn)(void);
void level_lighting_register(int level_num, LevelLightingFn fn);
void level_lighting_apply_current(void);
void level_lights_compute_shadow_vp_sun(int idx,
                                        float scene_cx, float scene_cy, float scene_cz,
                                        float scene_radius);

void level_lights_set_fog(float r, float g, float b, float near_dist, float far_dist, float max_density);
void level_lights_fill_fog_cb(FogCBData *out);

void level_lights_set_override(int on);
int  level_lights_get_override(void);

#ifdef __cplusplus
}
#endif
