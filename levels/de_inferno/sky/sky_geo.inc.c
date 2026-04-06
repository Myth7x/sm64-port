#include "levels/de_inferno/sky/sky_model.inc.c"
#include "levels/de_inferno/sky/sky_camera.inc.h"

/* forward declaration — defined in src/game/skybox3d.c */
void sky3d_register(Gfx *dl, s32 ox, s32 oy, s32 oz, s32 scale);

static Gfx sky3d_display_list[] = {
    gsSPDisplayList(de_inferno_sky_dl_skybox_tidesbk_mesh_layer_1),
    gsSPDisplayList(de_inferno_sky_dl_skybox_tidesdn_mesh_layer_1),
    gsSPDisplayList(de_inferno_sky_dl_skybox_tidesft_mesh_layer_1),
    gsSPDisplayList(de_inferno_sky_dl_skybox_tideslf_mesh_layer_1),
    gsSPDisplayList(de_inferno_sky_dl_skybox_tidesrt_mesh_layer_1),
    gsSPDisplayList(de_inferno_sky_dl_skybox_tidesup_mesh_layer_1),
    gsSPDisplayList(de_inferno_sky_dl_nature_infblendgrassdirt001a_001_mesh_layer_1),
    gsSPDisplayList(de_inferno_sky_dl_nature_infskybox02_001_mesh_layer_1),
    gsSPDisplayList(de_inferno_sky_dl_nature_inftreelinea_mesh_layer_5),
    gsSPDisplayList(de_inferno_sky_dl_final_revert_mesh_layer_5),
    gsSPDisplayList(de_inferno_sky_dl_final_revert_mesh_layer_1),
    gsDPNoOpTag(0x534B5944u),  /* sky depth-clear marker (SKYD) */
    gsSPEndDisplayList(),
};

s32 de_inferno_sky_init(UNUSED s16 arg, UNUSED s32 unused) {
    sky3d_register(sky3d_display_list, SKY3D_ORIGIN_X, SKY3D_ORIGIN_Y, SKY3D_ORIGIN_Z, SKY3D_SCALE);
    return 0;
}
