#ifndef SOURCE_MOVEMENT_H
#define SOURCE_MOVEMENT_H

#include "types.h"

extern float gSrcMaxSpeed;
extern float gSrcAccel;
extern float gSrcAirAccel;
extern float gSrcAirWishCap;
extern float gSrcFriction;
extern float gSrcStopSpeed;
extern float gSrcJumpSpeed;
extern float gSrcBhopSpeedCap;
extern float gSrcWalkableNormal;
extern float gSrcSurfMinNormal;
extern float gSrcExtraGravity;
extern float gSrcTerminalVel;
extern float gSrcNoclipSpeed;
extern int   gSrcBhopWindow;

void src_fps_movement(struct MarioState *m);

#endif
