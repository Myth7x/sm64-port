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
#include "levels/bhop_crab_game/header.h"

/* Fast64 begin persistent block [scripts] */
/* Fast64 end persistent block [scripts] */

const LevelScript level_bhop_crab_game_entry[] = {
	INIT_LEVEL(),
	LOAD_MIO0(0x07, _bhop_crab_game_segment_7SegmentRomStart, _bhop_crab_game_segment_7SegmentRomEnd), 
	LOAD_MIO0(0x0A, _clouds_skybox_mio0SegmentRomStart, _clouds_skybox_mio0SegmentRomEnd), 
	ALLOC_LEVEL_POOL(),
	MARIO(MODEL_MARIO, 0x00000001, bhvMario), 
	/* Fast64 begin persistent block [level commands] */
	/* Fast64 end persistent block [level commands] */

	AREA(1, bhop_crab_game_area_1),    MARIO_POS(0x01, 0, 3200, 386, -1024),
		TERRAIN(bhop_crab_game_area_1_collision),
		MACRO_OBJECTS(bhop_crab_game_area_1_macro_objs),
		SET_BACKGROUND_MUSIC(0x00, SEQ_LEVEL_GRASS),
		TERRAIN_TYPE(TERRAIN_GRASS),
		/* Fast64 begin persistent block [area commands] */
		/* Fast64 end persistent block [area commands] */
		OBJECT(MODEL_NONE, -18560, 1960, -12352, 0, 0, 0, 0x00100000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -12800, 2000, -12352, 0, 0, 0, 0x00100000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -18560, 1928, -12352, 0, 0, 0, 0x00010000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -12800, 2000, -12352, 0, 0, 0, 0x00010000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -17696, 2480, -192, 0, 0, 0, 0x00020000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -17888, 2594, 1024, 0, 0, 0, 0x00020000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -4352, 1232, -1024, 0, 0, 0, 0x00030000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -7136, 1168, 64, 0, 0, 0, 0x00030000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -19544, 2124, 4736, 0, 0, 0, 0x00040000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -22452, 2594, 3520, 0, 0, 0, 0x00040000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -17792, 2880, -1408, 0, 0, 0, 0x00050000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -22452, 2594, 3520, 0, 0, 0, 0x00050000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -20808, 2128, -192, 0, 0, 0, 0x00060000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -17888, 2594, 1024, 0, 0, 0, 0x00060000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -22560, 2880, 5952, 0, 0, 0, 0x00070000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -12800, 2000, -12352, 0, 0, 0, 0x00070000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -18560, 208, -12352, 0, 0, 0, 0x00080000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -12800, 2000, -12352, 0, 0, 0, 0x00080000, bhvInstantActiveWarp),
		OBJECT(MODEL_NONE, -18560, 1960, -12352, 0, 0, 0, 0x00090000, bhvInstantActiveWarp),
		WARP_NODE(0x10, LEVEL_BHOP_CRAB_GAME, 0x01, 0x10, 0x00),
		WARP_NODE(0x01, LEVEL_BHOP_CRAB_GAME, 0x01, 0x01, 0x00),
		WARP_NODE(0x02, LEVEL_BHOP_CRAB_GAME, 0x01, 0x02, 0x00),
		WARP_NODE(0x03, LEVEL_BHOP_CRAB_GAME, 0x01, 0x03, 0x00),
		WARP_NODE(0x04, LEVEL_BHOP_CRAB_GAME, 0x01, 0x04, 0x00),
		WARP_NODE(0x05, LEVEL_BHOP_CRAB_GAME, 0x01, 0x05, 0x00),
		WARP_NODE(0x06, LEVEL_BHOP_CRAB_GAME, 0x01, 0x06, 0x00),
		WARP_NODE(0x07, LEVEL_BHOP_CRAB_GAME, 0x01, 0x07, 0x00),
		WARP_NODE(0x08, LEVEL_BHOP_CRAB_GAME, 0x01, 0x08, 0x00),
		WARP_NODE(0x09, LEVEL_BHOP_CRAB_GAME, 0x01, 0x09, 0x00),


	END_AREA(),
	FREE_LEVEL_POOL(),    MARIO_POS(0x01, 0, 3200, 386, -1024),
	CALL(0, lvl_init_or_update),
		CALL(0, bhop_crab_game_entities_init),
	CALL_LOOP(1, lvl_init_or_update),
	CLEAR_LEVEL(),
	SLEEP_BEFORE_EXIT(1),
	EXIT(),
};