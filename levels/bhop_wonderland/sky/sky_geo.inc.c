#include "levels/bhop_wonderland/sky/sky_model.inc.c"
#include "levels/bhop_wonderland/sky/sky_camera.inc.h"

/* forward declaration — defined in src/game/skybox3d.c */
void sky3d_register(Gfx *dl, s32 ox, s32 oy, s32 oz, s32 scale);

static Gfx sky3d_display_list[] = {
    gsSPDisplayList(bhop_wonderland_sky_dl_alisa_alisablockverx_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_alisa_alisablockverx3_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_alisa_alisarock2_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_alisa_cobble_stone_b_blend1_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_alisa_rock03_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_alisa_son66_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_cobblestone_hr_c_inferno_cobblestone_a_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_cs_italy_door_sn01_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_cs_italy_hpe_cellar_stone_floor_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_cs_italy_hpe_plaster_wall_red_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_de_aztec_hr_aztec_hr_aztec_roof_stone_01_grey_color_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_de_aztec_hr_aztec_hr_aztec_wall_stone_01_moss_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_de_dust_cs_dust_square_window_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_de_dust_sitebwall08_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_dev_reflectivity_30b_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_devneons_green_neon_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_effects_xen_teleporter_refract_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_neon_my_kyst_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_neon_my_son1_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_neon_my_son2_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_neon_my_son3_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_neon_my_son4_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_neon_my_son5_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_realworldtextures2_tile_tile_01_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_stone_stone_floor_01_001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_stone_vostok_wall05_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_wood_houseceiling01_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_wood_houseceiling03_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_wood_milroof001_mesh_layer_1),
    gsSPDisplayList(bhop_wonderland_sky_dl_final_revert_mesh_layer_1),
    gsDPNoOpTag(0x534B5944u),  /* sky depth-clear marker (SKYD) */
    gsSPEndDisplayList(),
};

s32 bhop_wonderland_sky_init(UNUSED s16 arg, UNUSED s32 unused) {
    sky3d_register(sky3d_display_list, SKY3D_ORIGIN_X, SKY3D_ORIGIN_Y, SKY3D_ORIGIN_Z, SKY3D_SCALE);
    return 0;
}
