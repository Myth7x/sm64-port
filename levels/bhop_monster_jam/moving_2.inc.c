Vtx bhop_monster_jam_moving_2_vtx[4] = {
    {{{-96, 96, 16}, 0, {0, 0}, {0x80, 0x80, 0x80, 0xFF}}},
    {{{96, -96, 16}, 0, {0, 0}, {0x80, 0x80, 0x80, 0xFF}}},
    {{{96, 96, 16}, 0, {0, 0}, {0x80, 0x80, 0x80, 0xFF}}},
    {{{-96, -96, 16}, 0, {0, 0}, {0x80, 0x80, 0x80, 0xFF}}},
};

Gfx bhop_monster_jam_moving_2_dl[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPVertex(bhop_monster_jam_moving_2_vtx + 0, 4, 0),
    gsSP2Triangles(0, 1, 2, 0x0, 0, 3, 1, 0x0),
    gsSPEndDisplayList(),
};
