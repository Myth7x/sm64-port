#include <ultra64.h>
#include "sm64.h"
#include "behavior_data.h"
#include "model_ids.h"
#include "seq_ids.h"
#include "dialog_ids.h"
#include "segment_symbols.h"
#include "level_commands.h"

#include "game/level_update.h"

#include "levels/scripts.h"

#include "make_const_nonconst.h"
#include "levels/bhop_sky/header.h"

/* Fast64 begin persistent block [scripts] */
/* Fast64 end persistent block [scripts] */

const LevelScript level_bhop_sky_entry[] = {
	INIT_LEVEL(),
	LOAD_MIO0(0x07, _bhop_sky_segment_7SegmentRomStart, _bhop_sky_segment_7SegmentRomEnd), 
	LOAD_MIO0(0x0A, _water_skybox_mio0SegmentRomStart, _water_skybox_mio0SegmentRomEnd), 
	ALLOC_LEVEL_POOL(),
		LOAD_MODEL_FROM_DL(0xE2, bhop_sky_moving_0_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xE3, bhop_sky_moving_1_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xE4, bhop_sky_moving_2_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xE5, bhop_sky_moving_3_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xE6, bhop_sky_moving_4_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xE7, bhop_sky_moving_5_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xE8, bhop_sky_moving_6_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xE9, bhop_sky_moving_7_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xEA, bhop_sky_moving_8_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xEB, bhop_sky_moving_9_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xEC, bhop_sky_moving_10_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xED, bhop_sky_moving_11_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xEE, bhop_sky_moving_12_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xEF, bhop_sky_moving_13_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xF0, bhop_sky_moving_14_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xF1, bhop_sky_moving_15_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xF2, bhop_sky_moving_16_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xF3, bhop_sky_moving_17_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xF4, bhop_sky_moving_18_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xF5, bhop_sky_moving_19_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xF6, bhop_sky_moving_20_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xF7, bhop_sky_moving_21_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xF8, bhop_sky_moving_22_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xF9, bhop_sky_moving_23_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xFA, bhop_sky_moving_24_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xFB, bhop_sky_moving_25_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xFC, bhop_sky_moving_26_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xFD, bhop_sky_moving_27_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xFE, bhop_sky_moving_28_dl, LAYER_OPAQUE),
		LOAD_MODEL_FROM_DL(0xFF, bhop_sky_moving_29_dl, LAYER_OPAQUE),
	MARIO(MODEL_MARIO, 0x00000001, bhvMario), 
	/* Fast64 begin persistent block [level commands] */
	/* Fast64 end persistent block [level commands] */

	AREA(1, bhop_sky_area_1),    MARIO_POS(0x01, 0, 408, 16, 104),
		TERRAIN(bhop_sky_area_1_collision),
		MACRO_OBJECTS(bhop_sky_area_1_macro_objs),
		SET_BACKGROUND_MUSIC(0x00, SEQ_LEVEL_GRASS),
		TERRAIN_TYPE(TERRAIN_GRASS),
		/* Fast64 begin persistent block [area commands] */
		/* Fast64 end persistent block [area commands] */
		OBJECT(MODEL_NONE, -428, 100, 4064, 0, 0, 0, 0x00100000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 2744, 18, 240, 0, 0, 0, 0x00100000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -1188, -8, 228, 0, 0, 0, 0x00010000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 208, 18, 208, 0, 0, 0, 0x00010000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -3712, -8, 736, 0, 0, 0, 0x00020000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -2376, 18, 208, 0, 0, 0, 0x00020000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -2984, -8, 3564, 0, 0, 0, 0x00030000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -4280, 18, 2032, 0, 0, 0, 0x00030000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 2308, 100, 240, 0, 0, 0, 0x00040000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -840, 18, 4058, 0, 0, 0, 0x00040000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 4544, -8, -892, 0, 0, 0, 0x00050000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 2744, 18, 240, 0, 0, 0, 0x00050000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 6768, -8, -3956, 0, 0, 0, 0x00060000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 5323, 18, -2747, 0, 0, 0, 0x00060000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 9876, -8, -824, 0, 0, 0, 0x00070000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 8113, 18, -2380, 0, 0, 0, 0x00070000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 11640, 100, -3492, 0, 0, 0, 0x00080000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 320, 456, 224, 0, 0, 0, 0x00080000, bhvInstantActiveWarp),
		WARP_NODE(0x10, LEVEL_BHOP_SKY, 0x01, 0x10, 0x00),
		WARP_NODE(0x01, LEVEL_BHOP_SKY, 0x01, 0x01, 0x00),
		WARP_NODE(0x02, LEVEL_BHOP_SKY, 0x01, 0x02, 0x00),
		WARP_NODE(0x03, LEVEL_BHOP_SKY, 0x01, 0x03, 0x00),
		WARP_NODE(0x04, LEVEL_BHOP_SKY, 0x01, 0x04, 0x00),
		WARP_NODE(0x05, LEVEL_BHOP_SKY, 0x01, 0x05, 0x00),
		WARP_NODE(0x06, LEVEL_BHOP_SKY, 0x01, 0x06, 0x00),
		WARP_NODE(0x07, LEVEL_BHOP_SKY, 0x01, 0x07, 0x00),
		WARP_NODE(0x08, LEVEL_BHOP_SKY, 0x01, 0x08, 0x00),
		OBJECT(0xE2, -1424, -8, 96, 0, 0, 0, 0x00000000, bhvCustomMovingPlatform),
		OBJECT(0xE3, -3072, -8, 104, 0, 0, 0, 0x00010000, bhvCustomMovingPlatform),
		OBJECT(0xE4, -3416, -8, 344, 0, 0, 0, 0x00020000, bhvCustomMovingPlatform),
		OBJECT(0xE5, -3792, -8, 208, 0, 0, 0, 0x00030000, bhvCustomMovingPlatform),
		OBJECT(0xE6, -4120, -8, 440, 0, 0, 0, 0x00040000, bhvCustomMovingPlatform),
		OBJECT(0xE7, -4256, -8, 856, 0, 0, 0, 0x00050000, bhvCustomMovingPlatform),
		OBJECT(0xE8, -4336, -8, 2720, 0, 0, 0, 0x00060000, bhvCustomMovingPlatform),
		OBJECT(0xE9, -4088, -8, 3056, 0, 0, 0, 0x00070000, bhvCustomMovingPlatform),
		OBJECT(0xEA, -4096, -8, 3504, 0, 0, 0, 0x00080000, bhvCustomMovingPlatform),
		OBJECT(0xEB, -3936, -8, 3952, 0, 0, 0, 0x00090000, bhvCustomMovingPlatform),
		OBJECT(0xEC, -3576, -8, 4272, 0, 0, 0, 0x000A0000, bhvCustomMovingPlatform),
		OBJECT(0xED, -3136, -8, 4384, 0, 0, 0, 0x000B0000, bhvCustomMovingPlatform),
		OBJECT(0xEE, -1936, -8, 4056, 0, 0, 0, 0x000C0000, bhvCustomMovingPlatform),
		OBJECT(0xEF, -1528, -8, 4072, 0, 0, 0, 0x000D0000, bhvCustomMovingPlatform),
		OBJECT(0xF0, 3432, -8, 128, 0, 0, 0, 0x000E0000, bhvCustomMovingPlatform),
		OBJECT(0xF1, 3744, -8, 328, 0, 0, 0, 0x000F0000, bhvCustomMovingPlatform),
		OBJECT(0xF2, 4096, -8, 176, 0, 0, 0, 0x00100000, bhvCustomMovingPlatform),
		OBJECT(0xF3, 4488, -8, 272, 0, 0, 0, 0x00110000, bhvCustomMovingPlatform),
		OBJECT(0xF4, 4888, -8, 136, 0, 0, 0, 0x00120000, bhvCustomMovingPlatform),
		OBJECT(0xF5, 5216, -8, -88, 0, 0, 0, 0x00130000, bhvCustomMovingPlatform),
		OBJECT(0xF6, 5528, -8, -384, 0, 0, 0, 0x00140000, bhvCustomMovingPlatform),
		OBJECT(0xF7, 5328, -8, -1616, 0, 0, 0, 0x00150000, bhvCustomMovingPlatform),
		OBJECT(0xF8, 5312, -8, -2096, 0, 0, 0, 0x00160000, bhvCustomMovingPlatform),
		OBJECT(0xF9, 5240, -8, -3600, 0, 0, 0, 0x00170000, bhvCustomMovingPlatform),
		OBJECT(0xFA, 5368, -8, -4016, 0, 0, 0, 0x00180000, bhvCustomMovingPlatform),
		OBJECT(0xFB, 5584, -8, -4416, 0, 0, 0, 0x00190000, bhvCustomMovingPlatform),
		OBJECT(0xFC, 5944, -8, -4656, 0, 0, 0, 0x001A0000, bhvCustomMovingPlatform),
		OBJECT(0xFD, 6304, -8, -4816, 0, 0, 0, 0x001B0000, bhvCustomMovingPlatform),
		OBJECT(0xFE, 6728, -8, -4872, 0, 0, 0, 0x001C0000, bhvCustomMovingPlatform),
		OBJECT(0xFF, 7232, -8, -4800, 0, 0, 0, 0x001D0000, bhvCustomMovingPlatform),



	END_AREA(),
	FREE_LEVEL_POOL(),    MARIO_POS(0x01, 0, 408, 16, 104),
	CALL(0, lvl_init_or_update),
		CALL(0, bhop_sky_entities_init),
	CALL_LOOP(1, lvl_init_or_update),
	CLEAR_LEVEL(),
	SLEEP_BEFORE_EXIT(1),
	EXIT(),
};