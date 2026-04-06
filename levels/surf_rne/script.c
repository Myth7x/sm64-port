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
#include "levels/surf_rne/header.h"

/* Fast64 begin persistent block [scripts] */
/* Fast64 end persistent block [scripts] */

const LevelScript level_surf_rne_entry[] = {
	INIT_LEVEL(),
	LOAD_MIO0(0x07, _surf_rne_segment_7SegmentRomStart, _surf_rne_segment_7SegmentRomEnd), 
	LOAD_MIO0(0x0A, _clouds_skybox_mio0SegmentRomStart, _clouds_skybox_mio0SegmentRomEnd), 
	ALLOC_LEVEL_POOL(),
	MARIO(MODEL_MARIO, 0x00000001, bhvMario),
	/* Fast64 begin persistent block [level commands] */
	/* Fast64 end persistent block [level commands] */

	AREA(1, surf_rne_area_1),    MARIO_POS(0x01, 0, 7632, 2, -23960),
		TERRAIN(surf_rne_area_1_collision),
		MACRO_OBJECTS(surf_rne_area_1_macro_objs),
		SET_BACKGROUND_MUSIC(0x00, SEQ_LEVEL_GRASS),
		TERRAIN_TYPE(TERRAIN_GRASS),
		/* Fast64 begin persistent block [area commands] */
		/* Fast64 end persistent block [area commands] */
		OBJECT(MODEL_NONE, 13824, -1407, -24564, 0, 0, 0, 0x00100000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 7936, 2, -24576, 0, 0, 0, 0x00100000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 19438, -1009, -24972, 0, 0, 0, 0x00010000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -12160, 8962, -23552, 0, 0, 0, 0x00010000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -9184, 3105, -23552, 0, 0, 0, 0x00020000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -12160, 8962, -23552, 0, 0, 0, 0x00020000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -16448, -7615, -7168, 0, 0, 0, 0x00030000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -7936, -4094, -7168, 0, 0, 0, 0x00030000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -18298, 2511, -23552, 0, 0, 0, 0x00040000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -7936, -4094, -7168, 0, 0, 0, 0x00040000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -23932, -6961, -7168, 0, 0, 0, 0x00050000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -22144, -13566, 19808, 0, 0, 0, 0x00050000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -10104, -15728, 19808, 0, 0, 0, 0x00060000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -22144, -13566, 19808, 0, 0, 0, 0x00060000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -1206, -14065, 19808, 0, 0, 0, 0x00070000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -22016, 16386, 4864, 0, 0, 0, 0x00070000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 27856, 17592, -1744, 0, 0, 0, 0x00080000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 0, 2, 0, 0, 0, 0, 0x00080000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 14080, 17313, -4160, 0, 0, 0, 0x00090000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 2848, 22530, -2944, 0, 0, 0, 0x00090000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 13600, 5185, -16384, 0, 0, 0, 0x000a0000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 13184, 9538, -16384, 0, 0, 0, 0x000a0000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 19724, -26508, 15502, 0, 0, 0, 0x000b0000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 11332, -19084, 12654, 0, 0, 0, 0x000b0000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 26624, -14207, -17696, 0, 0, 0, 0x000c0000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 26624, -10238, -25216, 0, 0, 0, 0x000c0000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -504, 144, 0, 0, 0, 0, 0x000d0000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 7936, 2, -24576, 0, 0, 0, 0x000d0000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -10216, 15519, 4848, 0, 0, 0, 0x000e0000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 2848, 22530, -2944, 0, 0, 0, 0x000e0000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -16384, 15193, 4864, 0, 0, 0, 0x000f0000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -22016, 16386, 4864, 0, 0, 0, 0x000f0000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 27852, 17587, -1744, 0, 0, 0, 0x00000000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -4096, 2, 0, 0, 0, 0, 0x00000000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -4600, 144, 0, 0, 0, 0, 0x00010000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 26624, -10238, -25216, 0, 0, 0, 0x00010000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 26626, -16433, -3460, 0, 0, 0, 0x00020000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 13184, 9538, -16384, 0, 0, 0, 0x00020000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 12480, 6287, -15364, 0, 0, 0, 0x00030000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 11332, -19084, 12654, 0, 0, 0, 0x00030000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 12480, 6287, -17404, 0, 0, 0, 0x00040000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 11332, -19084, 12654, 0, 0, 0, 0x00040000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 30504, -26879, 17006, 0, 0, 0, 0x00050000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, 0, 2, 0, 0, 0, 0, 0x00050000, bhvInstantActiveWarp),
		WARP_NODE(0x10, LEVEL_SURF_RNE, 0x01, 0x10, 0x00),
		WARP_NODE(0x01, LEVEL_SURF_RNE, 0x01, 0x01, 0x00),
		WARP_NODE(0x02, LEVEL_SURF_RNE, 0x01, 0x02, 0x00),
		WARP_NODE(0x03, LEVEL_SURF_RNE, 0x01, 0x03, 0x00),
		WARP_NODE(0x04, LEVEL_SURF_RNE, 0x01, 0x04, 0x00),
		WARP_NODE(0x05, LEVEL_SURF_RNE, 0x01, 0x05, 0x00),
		WARP_NODE(0x06, LEVEL_SURF_RNE, 0x01, 0x06, 0x00),
		WARP_NODE(0x07, LEVEL_SURF_RNE, 0x01, 0x07, 0x00),
		WARP_NODE(0x08, LEVEL_SURF_RNE, 0x01, 0x08, 0x00),
		WARP_NODE(0x09, LEVEL_SURF_RNE, 0x01, 0x09, 0x00),
		WARP_NODE(0x0A, LEVEL_SURF_RNE, 0x01, 0x0A, 0x00),
		WARP_NODE(0x0B, LEVEL_SURF_RNE, 0x01, 0x0B, 0x00),
		WARP_NODE(0x0C, LEVEL_SURF_RNE, 0x01, 0x0C, 0x00),
		WARP_NODE(0x0D, LEVEL_SURF_RNE, 0x01, 0x0D, 0x00),
		WARP_NODE(0x0E, LEVEL_SURF_RNE, 0x01, 0x0E, 0x00),
		WARP_NODE(0x0F, LEVEL_SURF_RNE, 0x01, 0x0F, 0x00),
		WARP_NODE(0x00, LEVEL_SURF_RNE, 0x01, 0x00, 0x00),
		WARP_NODE(0x01, LEVEL_SURF_RNE, 0x01, 0x01, 0x00),
		WARP_NODE(0x02, LEVEL_SURF_RNE, 0x01, 0x02, 0x00),
		WARP_NODE(0x03, LEVEL_SURF_RNE, 0x01, 0x03, 0x00),
		WARP_NODE(0x04, LEVEL_SURF_RNE, 0x01, 0x04, 0x00),
		WARP_NODE(0x05, LEVEL_SURF_RNE, 0x01, 0x05, 0x00),


	END_AREA(),
	FREE_LEVEL_POOL(),    MARIO_POS(0x01, 0, 7632, 2, -23960),
	CALL(0, lvl_init_or_update),
		CALL(0, surf_rne_entities_init),
	CALL_LOOP(1, lvl_init_or_update),
	CLEAR_LEVEL(),
	SLEEP_BEFORE_EXIT(1),
	EXIT(),
};