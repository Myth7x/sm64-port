#include "levels/de_nuke/sky/sky_model.inc.c"
#include "levels/de_nuke/sky/sky_camera.inc.h"

/* forward declaration — defined in src/game/skybox3d.c */
void sky3d_register(Gfx *dl, s32 ox, s32 oy, s32 oz, s32 scale);

static Gfx sky3d_display_list[] = {
    gsSPDisplayList(de_nuke_sky_dl_de_nuke_nuksky01_mesh_layer_5),
    gsSPDisplayList(de_nuke_sky_dl_de_nuke_nuksky02_mesh_layer_5),
    gsSPDisplayList(de_nuke_sky_dl_de_nuke_nuksky03_mesh_layer_5),
    gsSPDisplayList(de_nuke_sky_dl_de_nuke_nuksky04_mesh_layer_5),
    gsSPDisplayList(de_nuke_sky_dl_final_revert_mesh_layer_5),
    gsSPDisplayList(de_nuke_sky_dl_skybox_de_cobble_hdrbk_mesh_layer_1),
    gsSPDisplayList(de_nuke_sky_dl_skybox_de_cobble_hdrdn_mesh_layer_1),
    gsSPDisplayList(de_nuke_sky_dl_skybox_de_cobble_hdrft_mesh_layer_1),
    gsSPDisplayList(de_nuke_sky_dl_skybox_de_cobble_hdrlf_mesh_layer_1),
    gsSPDisplayList(de_nuke_sky_dl_skybox_de_cobble_hdrrt_mesh_layer_1),
    gsSPDisplayList(de_nuke_sky_dl_skybox_de_cobble_hdrup_mesh_layer_1),
    gsSPDisplayList(de_nuke_sky_dl_final_revert_mesh_layer_1),
    gsDPNoOpTag(0x534B5944u),  /* sky depth-clear marker (SKYD) */
    gsSPEndDisplayList(),
};

s32 de_nuke_sky_init(UNUSED s16 arg, UNUSED s32 unused) {
    sky3d_register(sky3d_display_list, SKY3D_ORIGIN_X, SKY3D_ORIGIN_Y, SKY3D_ORIGIN_Z, SKY3D_SCALE);
    return 0;
}
