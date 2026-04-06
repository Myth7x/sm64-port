#include <math.h>

#include <ultra64.h>
#include "sm64.h"
#include "types.h"
#include "mario_step.h"
#include "engine/math_util.h"
#include "engine/surface_collision.h"
#include "fps_mode.h"
#include "fps_camera.h"
#include "source_movement.h"

#define DT (1.0f / 30.0f)
#define SPEED_EPSILON 0.01f
#define SRC_EYE_HEIGHT 128.0f

float gSrcMaxSpeed       = 30.0f;   /* sv_maxspeed 900 u/s / 30fps               */
float gSrcAccel          = 350.0f;  /* sv_accelerate — same value as reference     */
float gSrcAirAccel       = 350.0f;  /* sv_airaccelerate — CSS surf standard        */
float gSrcAirWishCap     = 6.0f;    /* per-strafe cap: 30 u/s / 30fps = 1 u/frame  */
float gSrcFriction       = 4.0f;    /* sv_friction                                 */
float gSrcStopSpeed      = 3.33f;   /* sv_stopspeed 100 u/s / 30fps                */
float gSrcJumpSpeed      = 42.0f;   /* SM64 native jump (≈ ref arc timing at 30fps) */
float gSrcBhopSpeedCap   = 120.0f;  /* max horizontal speed in air                 */
float gSrcWalkableNormal = 0.7f;
float gSrcSurfMinNormal  = 0.001f;  /* ramp threshold — match reference 0.001      */
float gSrcExtraGravity   = 0.0f;    /* SM64's 4 u/frame base matches ref arc time  */
float gSrcTerminalVel    = -160.0f;
float gSrcNoclipSpeed    = 40.0f;
int   gSrcBhopWindow     = 4;

/* -----------------------------------------------------------------------
 * Per-frame persistent state
 * ----------------------------------------------------------------------- */
static bool sWasGrounded = FALSE;
static bool sOnSurf = FALSE;
static bool sBhopBuffered = FALSE;
static int  sBhopTimer    = 0;

/* -----------------------------------------------------------------------
 * Internal helpers
 * ----------------------------------------------------------------------- */

/**
 * Apply Source-style ground friction to the horizontal velocity components.
 *
 * Formula (matching Valve's Half-Life / Source SDK implementation):
 *   speed        = |vel_xz|
 *   friction_mag = max(speed, SRC_STOPSPEED) * SRC_FRICTION * dt
 *   newSpeed     = max(0, speed - friction_mag)
 *   vel_xz       = vel_xz * (newSpeed / speed)
 */
static void src_apply_friction(struct MarioState *m) {
    f32 speed = sqrtf(m->vel[0] * m->vel[0] + m->vel[2] * m->vel[2]);
    if (speed < SPEED_EPSILON) {
        m->vel[0] = 0.0f;
        m->vel[2] = 0.0f;
        return;
    }

    f32 control        = (speed < gSrcStopSpeed) ? gSrcStopSpeed : speed;
    f32 frictionDelta  = control * gSrcFriction * DT;
    f32 newSpeed       = speed - frictionDelta;
    if (newSpeed < 0.0f) newSpeed = 0.0f;

    f32 scale = newSpeed / speed;
    m->vel[0] *= scale;
    m->vel[2] *= scale;
}

/*
 * Ground accelerate — Source-engine projection-cap formula with gainMultiplier.
 *
 * gainMultiplier = 1 - projVel/wishSpeed tapers the rate smoothly as velocity
 * approaches wishSpeed so you never overshoot.  When accel is large (350)
 * the raw addSpeed far exceeds the projCap (wishSpeed - projVel), so the cap
 * binds immediately and you snap to max speed in one frame — matching the
 * instant, crisp ground response of the reference.
 *
 *   projVel   = dot(vel_xz, wishDir)   (how much of vel is already along wish)
 *   gainMult  = 1 - projVel / wishSpeed
 *   addSpeed  = accel * wishSpeed * gainMult * dt
 *   addSpeed  = min(addSpeed, wishSpeed - projVel)   (ground cap = wishSpeed)
 *   vel_xz   += addSpeed * wishDir
 */
static void src_accelerate(struct MarioState *m,
                            f32 wishDirX, f32 wishDirZ,
                            f32 wishSpeed, f32 accel) {
    if (wishSpeed < SPEED_EPSILON) return;
    f32 projVel  = m->vel[0] * wishDirX + m->vel[2] * wishDirZ;
    f32 gainMult = 1.0f - (projVel / wishSpeed);
    f32 addSpeed = accel * wishSpeed * gainMult * DT;
    if (projVel + addSpeed > wishSpeed)
        addSpeed = wishSpeed - projVel;
    if (addSpeed <= 0.0f)
        return;
    m->vel[0] += addSpeed * wishDirX;
    m->vel[2] += addSpeed * wishDirZ;
}

/*
 * Air accelerate — same gainMultiplier formula but the projection cap is
 * gSrcAirWishCap (1 u/frame = 30 u/s) instead of wishSpeed.
 *
 * The gainMultiplier uses wishSpeed (full maxspeed) as the denominator so the
 * taper rate is calibrated to the full speed range — matching the reference
 * Accelerate() where accelWishSpd = m_fMaxSpeed regardless of ground/air.
 * The final clamp is against gSrcAirWishCap so at most 1 u/frame of velocity
 * is added along wishDir per tick — the tight cap that creates skill-based
 * CSS surf air-strafing.
 *
 *   projVel   = dot(vel_xz, wishDir)
 *   gainMult  = 1 - projVel / wishSpeed          (uses full wishSpeed, not cap)
 *   addSpeed  = accel * wishSpeed * gainMult * dt
 *   addSpeed  = min(addSpeed, gSrcAirWishCap - projVel)   (air cap = 1 u/frame)
 *   vel_xz   += addSpeed * wishDir
 */
static void src_air_accelerate(struct MarioState *m,
                               f32 wishDirX, f32 wishDirZ,
                               f32 wishSpeed, f32 accel) {
    if (wishSpeed < SPEED_EPSILON) return;
    f32 projVel  = m->vel[0] * wishDirX + m->vel[2] * wishDirZ;
    f32 gainMult = 1.0f - (projVel / wishSpeed);
    f32 addSpeed = accel * wishSpeed * gainMult * DT;
    if (projVel + addSpeed > gSrcAirWishCap)
        addSpeed = gSrcAirWishCap - projVel;
    if (addSpeed <= 0.0f)
        return;
    m->vel[0] += addSpeed * wishDirX;
    m->vel[2] += addSpeed * wishDirZ;
}

/**
 * Source ClipVelocity (overbounce = 1.0):
 *   Remove the component of velocity pointing INTO the surface, leaving only
 *   the tangential component that slides along the surface.
 *
 *   vel_out = vel_in - dot(vel_in, normal) * normal
 *
 * This is the core of surfing: gravity pulls vel downward, which has a
 * component into the ramp.  ClipVelocity removes that each frame, redirecting
 * it along the ramp face.  The tangential component accumulates, accelerating
 * the player down the slope — exactly Source engine surf behaviour.
 *
 * Only clips when moving INTO the surface (backoff < 0) to avoid pulling
 * the player back when they are already separating from it.
 */
static void src_clip_velocity_along_surface(struct MarioState *m,
                                             f32 nx, f32 ny, f32 nz) {
    f32 backoff = m->vel[0]*nx + m->vel[1]*ny + m->vel[2]*nz;
    if (backoff < 0.0f) {
        m->vel[0] -= backoff * nx;
        m->vel[1] -= backoff * ny;
        m->vel[2] -= backoff * nz;
    }
}

/**
 * Clamp horizontal speed to maxSpeed (preserves direction).
 */
static void src_clamp_horizontal_speed(struct MarioState *m, f32 maxSpeed) {
    f32 speed = sqrtf(m->vel[0] * m->vel[0] + m->vel[2] * m->vel[2]);
    if (speed > maxSpeed && speed > SPEED_EPSILON) {
        f32 scale = maxSpeed / speed;
        m->vel[0] *= scale;
        m->vel[2] *= scale;
    }
}

/* -----------------------------------------------------------------------
 * Movement modes
 * ----------------------------------------------------------------------- */

/*
 * Ground movement — matches Source SDK FullWalkMove order:
 *   1. CheckJumpButton first (sets grounded=false, skipping friction entirely).
 *   2. Friction only if not jumping.
 *   3. Accelerate (only if not jumping — reference skips accel on jump frame too).
 *   4. Clamp to maxSpeed / perform ground step.
 *
 * Bhop: landing frame with A held OR sBhopBuffered → jump skips friction,
 * carrying full aerial speed.  Manual-buffer bhop fires even if A is released
 * on the landing frame (window = gSrcBhopWindow frames, set in src_air_step).
 */
static void src_ground_step(struct MarioState *m) {
    f32 mag      = m->intendedMag / 32.0f;
    f32 wishDirX = sins(m->intendedYaw);
    f32 wishDirZ = coss(m->intendedYaw);
    f32 wishSpeed = mag * gSrcMaxSpeed;

    bool aHeld      = (m->controller->buttonDown & A_BUTTON) != 0;
    bool justLanded = !sWasGrounded;
    bool shouldJump = aHeld || (justLanded && sBhopBuffered);

    if (shouldJump) {
        sBhopBuffered = FALSE;
        sBhopTimer    = 0;
        m->vel[1] = gSrcJumpSpeed;
        m->vel[1] -= gSrcExtraGravity;
        if (m->vel[1] < gSrcTerminalVel) m->vel[1] = gSrcTerminalVel;
        perform_air_step(m, 0);
        sWasGrounded = FALSE;
        return;
    }

    if (justLanded) sBhopBuffered = FALSE;

    src_apply_friction(m);

    if (wishSpeed > SPEED_EPSILON) {
        src_accelerate(m, wishDirX, wishDirZ, wishSpeed, gSrcAirAccel);
    }

    src_clamp_horizontal_speed(m, gSrcMaxSpeed);

    s32 stepResult = perform_ground_step(m);
    if (stepResult == GROUND_STEP_LEFT_GROUND) {
        sWasGrounded = FALSE;
    } else {
        sWasGrounded = TRUE;
        if (m->vel[1] < 0.0f) m->vel[1] = 0.0f;
    }
}

/**
 * Air movement: air-strafe acceleration (no friction) → gravity → SM64 collision step.
 *
 * --- Normal air (sOnSurf == FALSE) ---
 * perform_air_step() handles gravity and collision.  On landing we check the
 * floor normal: walkable (>= 0.7) → ground; steep surf ramp → enter surf path.
 *
 * --- Surf path (sOnSurf == TRUE) ---
 * Source's TryPlayerMove clips velocity along every surface it hits, letting the
 * player accelerate down a ramp as gravity is redirected along the face each tick.
 * SM64's perform_air_step does the opposite: on a floor hit it zeroes or clamps
 * vel[1], killing the along-ramp component and bleeding speed every frame.
 *
 * Fix: we own the velocity entirely when surfing.
 *  1. PRE-CLIP  — remove the into-ramp residual left by the previous frame.
 *  2. AirAccel  — add any player-input strafe.
 *  3. Gravity   — subtract 4 units manually (same as SM64's apply_gravity).
 *  4. POST-CLIP — redirect gravity energy along the ramp face (surf acceleration).
 *  5. Save the resulting desired velocity (vx, vy, vz).
 *  6. Pre-cancel internal gravity: add +4 to vel[1] so that when perform_air_step
 *     subtracts 4 internally, the net change is 0 and pos advances with our vel.
 *  7. Call perform_air_step solely for position integration and floor detection.
 *  8. Restore our saved velocity — discard whatever SM64 did to it.
 */

#define CEIL_PROBE_RADIUS 50.0f

static f32 find_min_ceil(f32 x, f32 qy, f32 z, struct Surface **pceil) {
    struct Surface *c;
    f32 best = CELL_HEIGHT_LIMIT;
    *pceil = NULL;
    f32 h;
    h = find_ceil(x,                  qy, z,                  &c); if (h < best) { best = h; *pceil = c; }
    h = find_ceil(x + CEIL_PROBE_RADIUS, qy, z,               &c); if (h < best) { best = h; *pceil = c; }
    h = find_ceil(x - CEIL_PROBE_RADIUS, qy, z,               &c); if (h < best) { best = h; *pceil = c; }
    h = find_ceil(x, qy, z + CEIL_PROBE_RADIUS,               &c); if (h < best) { best = h; *pceil = c; }
    h = find_ceil(x, qy, z - CEIL_PROBE_RADIUS,               &c); if (h < best) { best = h; *pceil = c; }
    return best;
}

static void apply_ceil_block(struct MarioState *m, f32 ceilH, struct Surface *ceil) {
    if (m->pos[1] + SRC_EYE_HEIGHT < ceilH) return;
    m->pos[1] = ceilH - SRC_EYE_HEIGHT;
    if (ceil != NULL) {
        f32 backoff = m->vel[0]*ceil->normal.x + m->vel[1]*ceil->normal.y + m->vel[2]*ceil->normal.z;
        if (backoff < 0.0f) {
            m->vel[0] -= backoff * ceil->normal.x;
            m->vel[1] -= backoff * ceil->normal.y;
            m->vel[2] -= backoff * ceil->normal.z;
        }
    } else {
        if (m->vel[1] > 0.0f) m->vel[1] = 0.0f;
    }
}
static void src_air_step(struct MarioState *m) {
    if (m->controller->buttonDown & A_BUTTON) {
        sBhopBuffered = TRUE;
        sBhopTimer    = gSrcBhopWindow;
    } else if (sBhopTimer > 0) {
        sBhopTimer--;
        if (sBhopTimer == 0) {
            sBhopBuffered = FALSE;
        }
    }

    f32 mag      = m->intendedMag / 32.0f;
    f32 wishDirX = sins(m->intendedYaw);
    f32 wishDirZ = coss(m->intendedYaw);
    f32 wishSpeed = mag * gSrcMaxSpeed;

    if (sOnSurf && m->floor != NULL) {
        f32 nx = m->floor->normal.x;
        f32 ny = m->floor->normal.y;
        f32 nz = m->floor->normal.z;

        src_clip_velocity_along_surface(m, nx, ny, nz);

        if (mag > SPEED_EPSILON) {
            src_air_accelerate(m, wishDirX, wishDirZ, wishSpeed, gSrcAirAccel);
        }

        m->vel[1] -= 4.0f;
        if (m->vel[1] < gSrcTerminalVel) m->vel[1] = gSrcTerminalVel;

        src_clip_velocity_along_surface(m, nx, ny, nz);

        f32 vx = m->vel[0];
        f32 vy = m->vel[1];
        f32 vz = m->vel[2];

        f32 surfPrevY = m->pos[1];
        m->vel[1] += 4.0f;

        perform_air_step(m, 0);

        m->vel[0] = vx;
        m->vel[1] = vy;
        m->vel[2] = vz;

        {
            struct Surface *ceilSurf;
            f32 ceilH = find_min_ceil(m->pos[0], m->pos[1] + SRC_EYE_HEIGHT, m->pos[2], &ceilSurf);
            apply_ceil_block(m, ceilH, ceilSurf);
        }

        if (m->floor == NULL && vy < 0.0f) {
            struct Surface *sweepFloor;
            f32 sweepHeight = find_floor(m->pos[0], surfPrevY, m->pos[2], &sweepFloor);
            if (sweepFloor != NULL && sweepHeight >= m->pos[1] && sweepHeight <= surfPrevY) {
                m->floor       = sweepFloor;
                m->floorHeight = sweepHeight;
            }
        }

#define SURF_SNAP_DIST 40.0f

        if (m->floor != NULL
            && m->pos[1] <= m->floorHeight + SURF_SNAP_DIST
            && m->floor->normal.y >= gSrcSurfMinNormal
            && m->floor->normal.y < gSrcWalkableNormal) {
            m->pos[1] = m->floorHeight;
            src_clip_velocity_along_surface(m,
                m->floor->normal.x, m->floor->normal.y, m->floor->normal.z);
            sOnSurf      = TRUE;
            sWasGrounded = FALSE;
        } else if (m->floor != NULL
                && m->pos[1] <= m->floorHeight + SURF_SNAP_DIST
                && m->floor->normal.y >= gSrcWalkableNormal) {
            m->pos[1] = m->floorHeight;
            sOnSurf      = FALSE;
            if (m->vel[1] < 0.0f) m->vel[1] = 0.0f;
        } else {
            sOnSurf      = FALSE;
            sWasGrounded = FALSE;
        }
        return;
    }

    if (mag > SPEED_EPSILON) {
        src_air_accelerate(m, wishDirX, wishDirZ, wishSpeed, gSrcAirAccel);
    }

    src_clamp_horizontal_speed(m, gSrcBhopSpeedCap);

    m->vel[1] -= 4.0f + gSrcExtraGravity;
    if (m->vel[1] < gSrcTerminalVel) m->vel[1] = gSrcTerminalVel;

    // Pre-clip: clip velocity at current position to prevent entering walls this frame
    {
        struct WallCollisionData wd;
        wd.x = m->pos[0]; wd.y = m->pos[1]; wd.z = m->pos[2];
        wd.radius = 50.0f; wd.numWalls = 0; wd.offsetY = 150.0f;
        find_wall_collisions(&wd);
        for (s32 wi = 0; wi < wd.numWalls; wi++) {
            struct Surface *w = wd.walls[wi];
            f32 dot = m->vel[0]*w->normal.x + m->vel[2]*w->normal.z;
            if (dot < 0.0f) { m->vel[0] -= dot*w->normal.x; m->vel[2] -= dot*w->normal.z; }
        }
        wd.x = m->pos[0]; wd.y = m->pos[1]; wd.z = m->pos[2];
        wd.numWalls = 0; wd.offsetY = 30.0f;
        find_wall_collisions(&wd);
        for (s32 wi = 0; wi < wd.numWalls; wi++) {
            struct Surface *w = wd.walls[wi];
            f32 dot = m->vel[0]*w->normal.x + m->vel[2]*w->normal.z;
            if (dot < 0.0f) { m->vel[0] -= dot*w->normal.x; m->vel[2] -= dot*w->normal.z; }
        }
    }

    f32 prevY = m->pos[1];
    // 4 horizontal sub-steps so thin walls are never skipped at high speed
    for (s32 qi = 0; qi < 4; qi++) {
        m->pos[0] += m->vel[0] * 0.25f;
        m->pos[2] += m->vel[2] * 0.25f;
        struct WallCollisionData wd;
        wd.x = m->pos[0]; wd.y = m->pos[1]; wd.z = m->pos[2];
        wd.radius = 50.0f; wd.numWalls = 0; wd.offsetY = 150.0f;
        if (find_wall_collisions(&wd) > 0) {
            m->pos[0] = wd.x; m->pos[2] = wd.z;
            for (s32 wi = 0; wi < wd.numWalls; wi++) {
                struct Surface *w = wd.walls[wi];
                f32 dot = m->vel[0]*w->normal.x + m->vel[2]*w->normal.z;
                if (dot < 0.0f) { m->vel[0] -= dot*w->normal.x; m->vel[2] -= dot*w->normal.z; }
            }
        }
        wd.x = m->pos[0]; wd.y = m->pos[1]; wd.z = m->pos[2];
        wd.numWalls = 0; wd.offsetY = 30.0f;
        if (find_wall_collisions(&wd) > 0) {
            m->pos[0] = wd.x; m->pos[2] = wd.z;
            for (s32 wi = 0; wi < wd.numWalls; wi++) {
                struct Surface *w = wd.walls[wi];
                f32 dot = m->vel[0]*w->normal.x + m->vel[2]*w->normal.z;
                if (dot < 0.0f) { m->vel[0] -= dot*w->normal.x; m->vel[2] -= dot*w->normal.z; }
            }
        }
    }
    {
        f32 prevHeadY = m->pos[1] + SRC_EYE_HEIGHT;
        m->pos[1] += m->vel[1];
        struct Surface *ceil;
        f32 ceilH = find_min_ceil(m->pos[0], prevHeadY, m->pos[2], &ceil);
        apply_ceil_block(m, ceilH, ceil);
    }

    struct Surface *nextFloor;
    f32 nextFloorHeight = find_floor(m->pos[0], m->pos[1], m->pos[2], &nextFloor);

    if (nextFloor == NULL && m->vel[1] < 0.0f) {
        struct Surface *sweepFloor;
        f32 sweepHeight = find_floor(m->pos[0], prevY, m->pos[2], &sweepFloor);
        if (sweepFloor != NULL && sweepHeight >= m->pos[1] && sweepHeight <= prevY) {
            nextFloor       = sweepFloor;
            nextFloorHeight = sweepHeight;
        }
    }

    m->floor       = nextFloor;
    m->floorHeight = (nextFloor != NULL) ? nextFloorHeight : m->floorHeight;

    vec3f_copy(m->marioObj->header.gfx.pos, m->pos);
    vec3s_set(m->marioObj->header.gfx.angle, 0, m->faceAngle[1], 0);

    if (nextFloor != NULL && m->pos[1] <= nextFloorHeight && nextFloorHeight <= prevY) {
        m->pos[1] = nextFloorHeight;
        m->marioObj->header.gfx.pos[1] = nextFloorHeight;
        if (nextFloor->normal.y >= gSrcSurfMinNormal &&
            nextFloor->normal.y <  gSrcWalkableNormal) {
            src_clip_velocity_along_surface(m,
                nextFloor->normal.x, nextFloor->normal.y, nextFloor->normal.z);
            sOnSurf      = TRUE;
            sWasGrounded = FALSE;
        } else {
            sOnSurf = FALSE;
            if (m->vel[1] < 0.0f) m->vel[1] = 0.0f;
        }
    } else {
        sOnSurf      = FALSE;
        sWasGrounded = FALSE;
    }
}

/**
 * Noclip movement: fly freely through geometry at SRC_NOCLIP_SPEED.
 *
 * W/S moves along the camera's full 3D forward vector (pitch included).
 * A/D strafes horizontally perpendicular to yaw.
 * A-button (Space/Jump) ascends; Z-trigger (Ctrl) descends.
 */
static void src_noclip_step(struct MarioState *m) {
    f32 fwd    = m->controller->stickY / 64.0f;   /* +1 = W = forward */
    f32 strafe = m->controller->stickX / 64.0f;   /* +1 = D = right   */

    f32 cosP = coss(gFpsPitch);
    m->pos[0] += sins(gFpsYaw) * cosP  * fwd    * gSrcNoclipSpeed;
    m->pos[1] += sins(gFpsPitch)       * fwd    * gSrcNoclipSpeed;
    m->pos[2] += coss(gFpsYaw) * cosP  * fwd    * gSrcNoclipSpeed;

    m->pos[0] += -coss(gFpsYaw) * strafe * gSrcNoclipSpeed;
    m->pos[2] +=  sins(gFpsYaw) * strafe * gSrcNoclipSpeed;

    if (m->controller->buttonDown & A_BUTTON)  m->pos[1] += gSrcNoclipSpeed;
    if (m->controller->buttonDown & Z_TRIG)    m->pos[1] -= gSrcNoclipSpeed;

    m->vel[0] = m->vel[1] = m->vel[2] = 0.0f;

    vec3f_copy(m->marioObj->header.gfx.pos, m->pos);
    vec3s_set(m->marioObj->header.gfx.angle, 0, gFpsYaw, 0);

    sWasGrounded = FALSE;
}

/* -----------------------------------------------------------------------
 * Public entry point
 * ----------------------------------------------------------------------- */

void src_fps_movement(struct MarioState *m) {
    if (gNoclipMode) {
        src_noclip_step(m);
        return;
    }

    bool onGround = (m->floor != NULL)
                 && (m->pos[1] <= m->floorHeight + 4.0f)
                 && (m->floor->normal.y >= gSrcWalkableNormal);

    m->faceAngle[1] = gFpsYaw;

    if (onGround) {
        src_ground_step(m);
    } else {
        src_air_step(m);
    }
}
