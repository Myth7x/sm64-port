const Collision bhop_monster_jam_moving_2_area_1_collision[] = {
	COL_INIT(),
	COL_VERTEX_INIT(4),
	COL_VERTEX(-96, 96, 16),
	COL_VERTEX(96, -96, 16),
	COL_VERTEX(96, 96, 16),
	COL_VERTEX(-96, -96, 16),
	COL_TRI_INIT(SURFACE_DEFAULT, 2),
	COL_TRI(0, 1, 2),
	COL_TRI(0, 3, 1),
	COL_TRI_STOP(),
	COL_END()
};
