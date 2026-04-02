#include <ultra64.h>

#include "paintings.h"
#include "surface_terrains.h"

const Gfx castle_grounds_seg7_dl_0700EA58[] = { gsSPEndDisplayList() };
const Gfx castle_grounds_seg7_us_dl_0700F2E8[] = { gsSPEndDisplayList() };
const Gfx dl_castle_lobby_wing_cap_light[] = { gsSPEndDisplayList() };
const Gfx dl_flying_carpet_begin[] = { gsSPEndDisplayList() };
const Gfx dl_flying_carpet_model_half[] = { gsSPEndDisplayList() };
const Gfx dl_flying_carpet_end[] = { gsSPEndDisplayList() };

u8 removed_level_u8_stub[] = { 0, 0, 0, 0 };
s16 removed_level_s16_stub[] = { 0, 0, 0, 0, -1 };
Gfx removed_level_gfx_stub[] = { gsSPEndDisplayList() };

const s16 flying_carpet_static_vertex_data[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

s16 ttc_movtex_tris_big_surface_treadmill[] = { 0, -1 };
s16 ttc_movtex_tris_small_surface_treadmill[] = { 0, -1 };

const Collision wf_seg7_collision_rotating_platform[] = {
    COL_INIT(),
    COL_VERTEX_INIT(0),
    COL_TRI_INIT(SURFACE_DEFAULT, 0),
    COL_TRI_STOP(),
    COL_END(),
};

const Collision wdw_seg7_collision_070186B4[] = {
    COL_INIT(),
    COL_VERTEX_INIT(0),
    COL_TRI_INIT(SURFACE_DEFAULT, 0),
    COL_TRI_STOP(),
    COL_END(),
};

const Collision wdw_seg7_collision_07018528[] = {
    COL_INIT(),
    COL_VERTEX_INIT(0),
    COL_TRI_INIT(SURFACE_DEFAULT, 0),
    COL_TRI_STOP(),
    COL_END(),
};

const Collision wf_seg7_collision_tumbling_bridge[] = {
    COL_INIT(),
    COL_VERTEX_INIT(0),
    COL_TRI_INIT(SURFACE_DEFAULT, 0),
    COL_TRI_STOP(),
    COL_END(),
};

const Collision bbh_seg7_collision_07026B1C[] = {
    COL_INIT(),
    COL_VERTEX_INIT(0),
    COL_TRI_INIT(SURFACE_DEFAULT, 0),
    COL_TRI_STOP(),
    COL_END(),
};

const Collision lll_seg7_collision_0701D21C[] = {
    COL_INIT(),
    COL_VERTEX_INIT(0),
    COL_TRI_INIT(SURFACE_DEFAULT, 0),
    COL_TRI_STOP(),
    COL_END(),
};

const Collision bitfs_seg7_collision_07015288[] = {
    COL_INIT(),
    COL_VERTEX_INIT(0),
    COL_TRI_INIT(SURFACE_DEFAULT, 0),
    COL_TRI_STOP(),
    COL_END(),
};

const Collision bowser_3_seg7_collision_07004B94[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};
const Collision bowser_3_seg7_collision_07004C18[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};
const Collision bowser_3_seg7_collision_07004C9C[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};
const Collision bowser_3_seg7_collision_07004D20[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};
const Collision bowser_3_seg7_collision_07004DA4[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};
const Collision bowser_3_seg7_collision_07004E28[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};
const Collision bowser_3_seg7_collision_07004EAC[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};
const Collision bowser_3_seg7_collision_07004F30[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};
const Collision bowser_3_seg7_collision_07004FB4[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};
const Collision bowser_3_seg7_collision_07005038[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};

const Collision inside_castle_seg7_collision_ddd_warp[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};
const Collision inside_castle_seg7_collision_ddd_warp_2[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};
const Collision bob_seg7_collision_gate[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};
const Collision hmc_seg7_collision_0702B65C[] = {
    COL_INIT(), COL_VERTEX_INIT(0), COL_TRI_INIT(SURFACE_DEFAULT, 0), COL_TRI_STOP(), COL_END(),
};

void *ccm_seg7_trajectory_snowman = NULL;
void *inside_castle_seg7_trajectory_mips = NULL;
const s16 removed_level_trajectory_stub[] = { -1 };

struct Painting bob_painting = { 0 };
struct Painting ccm_painting = { 0 };
struct Painting cotmc_painting = { 0 };
struct Painting wf_painting = { 0 };
struct Painting jrb_painting = { 0 };
struct Painting lll_painting = { 0 };
struct Painting ssl_painting = { 0 };
struct Painting hmc_painting = { 0 };
struct Painting ddd_painting = { 0 };
struct Painting wdw_painting = { 0 };
struct Painting thi_tiny_painting = { 0 };
struct Painting ttm_painting = { 0 };
struct Painting ttc_painting = { 0 };
struct Painting sl_painting = { 0 };
struct Painting thi_huge_painting = { 0 };
struct Painting ttm_slide_painting = { 0 };
