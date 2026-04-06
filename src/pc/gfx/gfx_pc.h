#ifndef GFX_PC_H
#define GFX_PC_H

#include <stdint.h>
#include <stdbool.h>

struct GfxRenderingAPI;
struct GfxWindowManagerAPI;

struct GfxDimensions {
    uint32_t width, height;
    float aspect_ratio;
};

extern struct GfxDimensions gfx_current_dimensions;

typedef struct {
    uint32_t texture_id;
    int16_t  width;
    int16_t  height;
} GfxTextureCacheEntry;

#ifdef __cplusplus
extern "C" {
#endif

void gfx_init(struct GfxWindowManagerAPI *wapi, struct GfxRenderingAPI *rapi, const char *game_name, bool start_in_fullscreen);
struct GfxRenderingAPI *gfx_get_current_rendering_api(void);
void gfx_start_frame(void);
void gfx_run(Gfx *commands);
void gfx_end_frame(void);
void gfx_imgui_frame(void);

uint32_t          gfx_texture_cache_count(void);
GfxTextureCacheEntry gfx_texture_cache_entry(uint32_t idx);
uintptr_t         gfx_get_imgui_tex_id(uint32_t texture_id);
void              gfx_pc_get_proj_matrix(float out[4][4]);
void              gfx_pc_get_ao_proj(float out[4][4]);

uint32_t          gfx_combiner_pool_count(void);
uint32_t          gfx_combiner_pool_cc_id(uint32_t idx);
extern uint32_t   gfx_dbg_draw_calls;
extern uint32_t   gfx_dbg_tris;

#ifdef __cplusplus
}
#endif

#endif
