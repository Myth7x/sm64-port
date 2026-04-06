#ifndef BHOP_SKY_HEADER_H
#define BHOP_SKY_HEADER_H

#include "types.h"

extern const GeoLayout bhop_sky_area_1_geo[];
extern const GeoLayout bhop_sky_area_1[];
extern const Collision bhop_sky_area_1_collision[];
extern const MacroObject bhop_sky_area_1_macro_objs[];
extern Lights1 bhop_sky_dl_concrete_concretefloor001a_001_lights;
extern Lights1 bhop_sky_dl_concrete_concretefloor011a_001_lights;
extern Lights1 bhop_sky_dl_concrete_concretefloor026a_001_lights;
extern Lights1 bhop_sky_dl_glass_glasswindow070c_001_layer5_lights;
extern u8 bhop_sky_dl_concretefloor001a_rgba16[];
extern u8 bhop_sky_dl_concretefloor011a_rgba16[];
extern u8 bhop_sky_dl_concretefloor026a_rgba16[];
extern u8 bhop_sky_dl_glasswindow070c_rgba16[];
extern Vtx bhop_sky_dl_concrete_concretefloor001a_mesh_layer_1_vtx_cull[8];
extern Vtx bhop_sky_dl_concrete_concretefloor001a_mesh_layer_1_vtx_0[1228];
extern Gfx bhop_sky_dl_concrete_concretefloor001a_mesh_layer_1_tri_0[];
extern Vtx bhop_sky_dl_concrete_concretefloor011a_mesh_layer_1_vtx_cull[8];
extern Vtx bhop_sky_dl_concrete_concretefloor011a_mesh_layer_1_vtx_0[226];
extern Gfx bhop_sky_dl_concrete_concretefloor011a_mesh_layer_1_tri_0[];
extern Vtx bhop_sky_dl_concrete_concretefloor026a_mesh_layer_1_vtx_cull[8];
extern Vtx bhop_sky_dl_concrete_concretefloor026a_mesh_layer_1_vtx_0[10161];
extern Gfx bhop_sky_dl_concrete_concretefloor026a_mesh_layer_1_tri_0[];
extern Vtx bhop_sky_dl_glass_glasswindow070c_mesh_layer_5_vtx_cull[8];
extern Vtx bhop_sky_dl_glass_glasswindow070c_mesh_layer_5_vtx_0[178];
extern Gfx bhop_sky_dl_glass_glasswindow070c_mesh_layer_5_tri_0[];
extern Gfx mat_bhop_sky_dl_concrete_concretefloor001a_001[];
extern Gfx mat_revert_bhop_sky_dl_concrete_concretefloor001a_001[];
extern Gfx mat_bhop_sky_dl_concrete_concretefloor011a_001[];
extern Gfx mat_revert_bhop_sky_dl_concrete_concretefloor011a_001[];
extern Gfx mat_bhop_sky_dl_concrete_concretefloor026a_001[];
extern Gfx mat_revert_bhop_sky_dl_concrete_concretefloor026a_001[];
extern Gfx mat_bhop_sky_dl_glass_glasswindow070c_001_layer5[];
extern Gfx mat_revert_bhop_sky_dl_glass_glasswindow070c_001_layer5[];
extern Gfx bhop_sky_dl_concrete_concretefloor001a_mesh_layer_1[];
extern Gfx bhop_sky_dl_concrete_concretefloor011a_mesh_layer_1[];
extern Gfx bhop_sky_dl_concrete_concretefloor026a_mesh_layer_1[];
extern Gfx bhop_sky_dl_glass_glasswindow070c_mesh_layer_5[];
extern Gfx bhop_sky_dl_final_revert_mesh_layer_1[];
extern Gfx bhop_sky_dl_final_revert_mesh_layer_5[];

extern const LevelScript level_bhop_sky_entry[];

#include "game/entity_boxes.h"
extern const struct EntityBox bhop_sky_entity_boxes[];
s32 bhop_sky_entities_init(s16, s32);

#include "game/bhv_moving_platform.h"
extern Gfx bhop_sky_moving_0_dl[];
extern Gfx bhop_sky_moving_1_dl[];
extern Gfx bhop_sky_moving_2_dl[];
extern Gfx bhop_sky_moving_3_dl[];
extern Gfx bhop_sky_moving_4_dl[];
extern Gfx bhop_sky_moving_5_dl[];
extern Gfx bhop_sky_moving_6_dl[];
extern Gfx bhop_sky_moving_7_dl[];
extern Gfx bhop_sky_moving_8_dl[];
extern Gfx bhop_sky_moving_9_dl[];
extern Gfx bhop_sky_moving_10_dl[];
extern Gfx bhop_sky_moving_11_dl[];
extern Gfx bhop_sky_moving_12_dl[];
extern Gfx bhop_sky_moving_13_dl[];
extern Gfx bhop_sky_moving_14_dl[];
extern Gfx bhop_sky_moving_15_dl[];
extern Gfx bhop_sky_moving_16_dl[];
extern Gfx bhop_sky_moving_17_dl[];
extern Gfx bhop_sky_moving_18_dl[];
extern Gfx bhop_sky_moving_19_dl[];
extern Gfx bhop_sky_moving_20_dl[];
extern Gfx bhop_sky_moving_21_dl[];
extern Gfx bhop_sky_moving_22_dl[];
extern Gfx bhop_sky_moving_23_dl[];
extern Gfx bhop_sky_moving_24_dl[];
extern Gfx bhop_sky_moving_25_dl[];
extern Gfx bhop_sky_moving_26_dl[];
extern Gfx bhop_sky_moving_27_dl[];
extern Gfx bhop_sky_moving_28_dl[];
extern Gfx bhop_sky_moving_29_dl[];
#endif
