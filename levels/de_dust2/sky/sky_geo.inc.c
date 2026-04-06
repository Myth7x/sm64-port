#include "levels/de_dust2/sky/sky_model.inc.c"
#include "levels/de_dust2/sky/sky_camera.inc.h"

/* forward declaration — defined in src/game/skybox3d.c */
void sky3d_register(Gfx *dl, s32 ox, s32 oy, s32 oz, s32 scale);

static Gfx sky3d_display_list[] = {
    gsSPDisplayList(de_dust2_sky_dl_cs_italy_marketwall04_001_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_de_dust_groundsand03_001_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_de_dust_marketwall02_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_de_dust_marketwall02b_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_de_dust_marketwall03_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_de_dust_residbwall01_001_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_de_dust_residwall02_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_de_dust_residwall04_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_de_dust_sitebwall02_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_de_dust_sitebwall05_001_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_de_dust_sitebwall13_001_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_de_dust_stonewall02_001_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_de_dust_templewall04_001_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_skybox_sky_dustbk_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_skybox_sky_dustdn_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_skybox_sky_dustft_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_skybox_sky_dustlf_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_skybox_sky_dustrt_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_skybox_sky_dustup_mesh_layer_1),
    gsSPDisplayList(de_dust2_sky_dl_final_revert_mesh_layer_1),
    gsDPNoOpTag(0x534B5944u),  /* sky depth-clear marker (SKYD) */
    gsSPEndDisplayList(),
};

s32 de_dust2_sky_init(UNUSED s16 arg, UNUSED s32 unused) {
    sky3d_register(sky3d_display_list, SKY3D_ORIGIN_X, SKY3D_ORIGIN_Y, SKY3D_ORIGIN_Z, SKY3D_SCALE);
    return 0;
}
