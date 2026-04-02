#ifndef SOURCE_MOVEMENT_H
#define SOURCE_MOVEMENT_H

#include "types.h"

/**
 * Main entry point for the Source-engine-style movement system.
 *
 * Handles ground friction, acceleration, air strafing, bunny hopping, and
 * noclip.  Must be called AFTER update_mario_inputs() so that m->intendedYaw,
 * m->intendedMag, and m->input are current.
 *
 * Calls SM64's perform_ground_step() / perform_air_step() internally so that
 * wall/floor collision detection still works.
 */
void src_fps_movement(struct MarioState *m);

#endif
