#include "level_lights.h"
#include <string.h>
#include <math.h>

static LevelLight s_lights[LEVEL_LIGHTS_MAX];
static int        s_light_count;
static float      s_ambient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
static float      s_shadow_vp[LEVEL_SHADOW_MAX][16];
static int        s_shadow_count;
static float      s_sun_dir[4]   = { 0.0f, 1.0f, 0.0f, 0.0f };
static float      s_sun_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

static float s_fog_color[3]  = { 0.5f, 0.5f, 0.5f };
static float s_fog_near      = 1000.0f;
static float s_fog_far       = 10000.0f;
static float s_fog_max_density = 1.0f;
static int   s_fog_enabled   = 0;
static int   s_override      = 0;

void level_lights_clear(void) {
    s_light_count  = 0;
    s_shadow_count = 0;
    s_ambient[0] = 1.0f;
    s_ambient[1] = 1.0f;
    s_ambient[2] = 1.0f;
    s_ambient[3] = 1.0f;
    s_fog_enabled = 0;
}

void level_lights_set_ambient(float r, float g, float b, float intensity) {
    s_ambient[0] = r;
    s_ambient[1] = g;
    s_ambient[2] = b;
    s_ambient[3] = intensity;
}

void level_lights_add(const LevelLight *light) {
    if (s_light_count >= LEVEL_LIGHTS_MAX) return;
    s_lights[s_light_count++] = *light;
}

int level_lights_count(void) {
    return s_light_count;
}

void level_lights_get(int idx, LevelLight *out) {
    if (idx < 0 || idx >= s_light_count) return;
    *out = s_lights[idx];
}

void level_lights_set(int idx, const LevelLight *light) {
    if (idx < 0 || idx >= s_light_count) return;
    s_lights[idx] = *light;
}

void level_lights_fill_cb(LightsCBData *out) {
    out->ambient_color[0] = s_ambient[0];
    out->ambient_color[1] = s_ambient[1];
    out->ambient_color[2] = s_ambient[2];
    out->ambient_color[3] = s_ambient[3];
    for (int i = 0; i < LEVEL_LIGHTS_MAX; i++) {
        if (i < s_light_count) {
            out->light_pos[i][0] = s_lights[i].position[0];
            out->light_pos[i][1] = s_lights[i].position[1];
            out->light_pos[i][2] = s_lights[i].position[2];
            out->light_pos[i][3] = s_lights[i].radius;
            out->light_color[i][0] = s_lights[i].color[0];
            out->light_color[i][1] = s_lights[i].color[1];
            out->light_color[i][2] = s_lights[i].color[2];
            out->light_color[i][3] = s_lights[i].intensity;
        } else {
            memset(out->light_pos[i],   0, sizeof(out->light_pos[i]));
            memset(out->light_color[i], 0, sizeof(out->light_color[i]));
        }
    }
    for (int i = 0; i < LEVEL_SHADOW_MAX; i++) {
        memcpy(out->shadow_vp[i], s_shadow_vp[i], sizeof(s_shadow_vp[i]));
    }
    out->shadow_count = s_shadow_count;
    out->_pad[0] = out->_pad[1] = out->_pad[2] = 0.0f;
    out->sun_dir[0] = s_sun_dir[0];
    out->sun_dir[1] = s_sun_dir[1];
    out->sun_dir[2] = s_sun_dir[2];
    out->sun_dir[3] = 0.0f;
    out->sun_color[0] = s_sun_color[0];
    out->sun_color[1] = s_sun_color[1];
    out->sun_color[2] = s_sun_color[2];
    out->sun_color[3] = 1.0f;
}

void level_lights_set_shadow_count(int n) {
    if (n < 0) n = 0;
    if (n > LEVEL_SHADOW_MAX) n = LEVEL_SHADOW_MAX;
    s_shadow_count = n;
}

void level_lights_set_sun(float dx, float dy, float dz, float r, float g, float b) {
    float len = sqrtf(dx*dx + dy*dy + dz*dz);
    if (len < 1e-6f) { dx = 0.0f; dy = 1.0f; dz = 0.0f; } else { dx/=len; dy/=len; dz/=len; }
    s_sun_dir[0] = dx; s_sun_dir[1] = dy; s_sun_dir[2] = dz; s_sun_dir[3] = 0.0f;
    s_sun_color[0] = r; s_sun_color[1] = g; s_sun_color[2] = b; s_sun_color[3] = 1.0f;
}

void level_lights_set_shadow_vp(int idx, const float mat4x4[16]) {
    if (idx < 0 || idx >= LEVEL_SHADOW_MAX) return;
    memcpy(s_shadow_vp[idx], mat4x4, 64);
}

static void mat4_mul(float out[16], const float a[16], const float b[16]) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            float s = 0.0f;
            for (int k = 0; k < 4; k++) s += a[r*4+k] * b[k*4+c];
            out[r*4+c] = s;
        }
    }
}

void level_lights_compute_shadow_vp(int idx, int light_idx,
                                    float cx, float cy, float cz,
                                    float scene_radius) {
    if (idx < 0 || idx >= LEVEL_SHADOW_MAX) return;
    if (light_idx < 0 || light_idx >= s_light_count) return;

    float ex = s_lights[light_idx].position[0];
    float ey = s_lights[light_idx].position[1];
    float ez = s_lights[light_idx].position[2];

    float dx = cx - ex, dy = cy - ey, dz = cz - ez;
    float dist = sqrtf(dx*dx + dy*dy + dz*dz);
    if (dist < 1.0f) dist = 1.0f;

    float zx = dx/dist, zy = dy/dist, zz = dz/dist;

    float ux = 0.0f, uy = 1.0f, uz = 0.0f;
    if (fabsf(zy) > 0.9f) { ux = 1.0f; uy = 0.0f; }

    float rx = uy*zz - uz*zy,
          ry = uz*zx - ux*zz,
          rz = ux*zy - uy*zx;
    float rl = sqrtf(rx*rx+ry*ry+rz*rz);
    if (rl < 1e-6f) { rx=1.0f; ry=rz=0.0f; } else { rx/=rl; ry/=rl; rz/=rl; }

    float upx = zy*rz - zz*ry,
          upy = zz*rx - zx*rz,
          upz = zx*ry - zy*rx;

    float V[16] = {
         rx,    ry,    rz,   -(rx*ex + ry*ey + rz*ez),
         upx,   upy,   upz,  -(upx*ex + upy*ey + upz*ez),
         zx,    zy,    zz,   -(zx*ex + zy*ey + zz*ez),
         0.0f,  0.0f,  0.0f,  1.0f
    };

    float R = scene_radius;
    float n = 0.1f, f = dist * 2.0f;
    float P[16] = {
        1.0f/R, 0.0f,   0.0f,           0.0f,
        0.0f,   1.0f/R, 0.0f,           0.0f,
        0.0f,   0.0f,   1.0f/(f-n),    -n/(f-n),
        0.0f,   0.0f,   0.0f,           1.0f
    };

    float VP[16];
    mat4_mul(VP, P, V);
    memcpy(s_shadow_vp[idx], VP, 64);
}

extern int16_t gCurrLevelNum;

#define LIGHTING_TABLE_SIZE 128
static LevelLightingFn s_lighting_fns[LIGHTING_TABLE_SIZE];

void level_lighting_register(int level_num, LevelLightingFn fn) {
    if (level_num > 0 && level_num < LIGHTING_TABLE_SIZE)
        s_lighting_fns[level_num] = fn;
}

void level_lights_set_override(int on) { s_override = on; }
int  level_lights_get_override(void)   { return s_override; }

void level_lighting_apply_current(void) {
    if (s_override) return;
    level_lights_clear();
    int n = gCurrLevelNum;
    if (n > 0 && n < LIGHTING_TABLE_SIZE && s_lighting_fns[n])
        s_lighting_fns[n]();
}

void level_lights_compute_shadow_vp_sun(int idx,
                                        float cx, float cy, float cz,
                                        float scene_radius) {
    if (idx < 0 || idx >= LEVEL_SHADOW_MAX) return;

    float dx = s_sun_dir[0], dy = s_sun_dir[1], dz = s_sun_dir[2];
    float dist = scene_radius * 3.0f;
    float ex = cx - dx * dist, ey = cy - dy * dist, ez = cz - dz * dist;

    float ux = 0.0f, uy = 1.0f, uz = 0.0f;
    if (fabsf(dy) > 0.9f) { ux = 1.0f; uy = 0.0f; uz = 0.0f; }

    float rx = uy*dz - uz*dy, ry = uz*dx - ux*dz, rz = ux*dy - uy*dx;
    float rl = sqrtf(rx*rx+ry*ry+rz*rz);
    if (rl < 1e-6f) { rx=1.0f; ry=rz=0.0f; } else { rx/=rl; ry/=rl; rz/=rl; }

    float upx = ry*dz - rz*dy, upy = rz*dx - rx*dz, upz = rx*dy - ry*dx;

    float V[16] = {
        rx,  ry,  rz,  -(rx*ex + ry*ey + rz*ez),
        upx, upy, upz, -(upx*ex + upy*ey + upz*ez),
        dx,  dy,  dz,  -(dx*ex + dy*ey + dz*ez),
        0.0f, 0.0f, 0.0f, 1.0f
    };

    float R = scene_radius;
    float n = 0.5f, f = dist * 2.0f + R;
    float P[16] = {
        1.0f/R, 0.0f,   0.0f,          0.0f,
        0.0f,   1.0f/R, 0.0f,          0.0f,
        0.0f,   0.0f,   1.0f/(f-n),   -n/(f-n),
        0.0f,   0.0f,   0.0f,          1.0f
    };

    float VP[16];
    mat4_mul(VP, P, V);
    memcpy(s_shadow_vp[idx], VP, 64);
}

void level_lights_set_fog(float r, float g, float b, float near_dist, float far_dist, float max_density) {
    s_fog_color[0] = r;
    s_fog_color[1] = g;
    s_fog_color[2] = b;
    s_fog_near = near_dist;
    s_fog_far  = far_dist;
    s_fog_max_density = max_density;
    s_fog_enabled = 1;
}

void level_lights_fill_fog_cb(FogCBData *out) {
    memset(out, 0, sizeof(*out));
    out->fog_color[0] = s_fog_color[0];
    out->fog_color[1] = s_fog_color[1];
    out->fog_color[2] = s_fog_color[2];
    out->fog_near        = s_fog_near;
    out->fog_far         = s_fog_far;
    out->fog_max_density = s_fog_max_density;
    out->fog_enabled     = s_fog_enabled ? 1.0f : 0.0f;
}
