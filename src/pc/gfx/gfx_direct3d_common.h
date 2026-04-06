#if defined(ENABLE_DX11) || defined(ENABLE_DX12)

#ifndef GFX_DIRECT3D_COMMON_H
#define GFX_DIRECT3D_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#include "gfx_cc.h"

struct AoCBData {
    float proj[16];
    float inv_proj[16];
    float rcp_size[2];
    float ao_radius;
    float ao_intensity;
    float ao_ambient[4];
    float _pad[24];
};

struct GfxSSAOParams {
    bool  enabled;
    bool  visualize;
    float radius;
    float intensity;
};

#ifdef __cplusplus
extern "C" {
#endif
GfxSSAOParams *gfx_ssao_params(void);
#ifdef __cplusplus
}
#endif

void gfx_direct3d_common_build_shader(char buf[16384], size_t& len, size_t& num_floats, const CCFeatures& cc_features, bool include_root_signature, bool three_point_filtering);
bool gfx_d3d_mat4_inverse(const float m[16], float out[16]);

#endif

#endif
