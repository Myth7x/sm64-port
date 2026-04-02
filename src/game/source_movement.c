#include <math.h>

#include <ultra64.h>
#include "sm64.h"
#include "types.h"
#include "mario_step.h"
#include "engine/math_util.h"
#include "fps_mode.h"
#include "fps_camera.h"
#include "source_movement.h"

/* -----------------------------------------------------------------------
 * Physics constants
 *
 * All speed values are in SM64 world-units per game frame (30 fps).
 * SM64 unit ≈ 1 cm, so:
 *   SRC_MAX_SPEED  64 u/frame  = 1920 u/s ≈ 19 m/s  (fast walk / slow run)
 *   SRC_JUMP_SPEED 68 u/frame  = vertical launch speed,  ~1.7 m/s² above SM64 normal
 *
 * The sv_friction / sv_accelerate numbers match Valve defaults and are applied
 * per-frame below using dt = 1/30.
 * ----------------------------------------------------------------------- */
#define DT (1.0f / 30.0f)
#define SRC_MAX_SPEED       24.0f
#define SRC_ACCEL           10.0f
#define SRC_AIR_ACCEL       10.0f
#define SRC_AIR_WISH_CAP    6.0f
#define SRC_FRICTION        2.5f
#define SRC_STOPSPEED       8.0f
#define SRC_JUMP_SPEED      42.0f
#define SRC_BHOP_SPEED_CAP  120.0f
#define SRC_WALKABLE_NORMAL  0.7f
#define SRC_SURF_MIN_NORMAL  0.05f
#define SRC_EXTRA_GRAVITY   0.8f
#define SRC_TERMINAL_VEL   -160.0f
#define SRC_NOCLIP_SPEED    140.0f
#define BHOP_WINDOW         4
#define SPEED_EPSILON       0.01f

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

    f32 control        = (speed < SRC_STOPSPEED) ? SRC_STOPSPEED : speed;
    f32 frictionDelta  = control * SRC_FRICTION * DT;
    f32 newSpeed       = speed - frictionDelta;
    if (newSpeed < 0.0f) newSpeed = 0.0f;

    f32 scale = newSpeed / speed;
    m->vel[0] *= scale;
    m->vel[2] *= scale;
}

/**
 * Accelerate toward wishDir at rate accel (Source sv_accelerate / sv_airaccelerate).
 *
 * Formula:
 *   currentSpeed = dot(vel_xz, wishDir)
 *   addSpeed     = wishSpeed - currentSpeed           // how much we lack
 *   if addSpeed <= 0: nothing to add
 *   accelSpeed   = min(addSpeed, accel * wishSpeed * dt)
 *   vel_xz      += accelSpeed * wishDir
 */
static void src_accelerate(struct MarioState *m,
                            f32 wishDirX, f32 wishDirZ,
                            f32 wishSpeed, f32 accel) {
    f32 currentSpeed = m->vel[0] * wishDirX + m->vel[2] * wishDirZ;
    f32 addSpeed     = wishSpeed - currentSpeed;
    if (addSpeed <= 0.0f) {
        return;
    }

    f32 accelSpeed = accel * wishSpeed * DT;
    if (accelSpeed > addSpeed) {
        accelSpeed = addSpeed;
    }

    m->vel[0] += accelSpeed * wishDirX;
    m->vel[2] += accelSpeed * wishDirZ;
}

/**
 * Source-engine AirAccelerate formula.
 *
 * Key difference from ground accelerate: the addSpeed cap uses a small
 * SRC_AIR_WISH_CAP instead of wishSpeed.  This means:
 *   - Even when total speed >> wishSpeed, the projection check still
 *     passes when strafing (projection ≈ 0), so you always get air accel.
 *   - The added vector is perpendicular-biased, which increases |vel| just
 *     like in Source — the speed gain is the geometric side-effect of adding
 *     a bounded perpendicular component to an already fast vector.
 *
 * Formula (matching Source SDK gamemovement.cpp AirAccelerate):
 *   wishSpd      = min(wishSpeed, SRC_AIR_WISH_CAP)   // sv_maxairspeed cap
 *   currentSpeed = dot(vel_xz, wishDir)                // projection
 *   addSpeed     = wishSpd - currentSpeed
 *   if addSpeed <= 0: return
 *   accelSpeed   = min(accel * wishSpeed * dt, addSpeed)  // uncapped wishSpeed
 *   vel_xz      += accelSpeed * wishDir
 */
static void src_air_accelerate(struct MarioState *m,
                               f32 wishDirX, f32 wishDirZ,
                               f32 wishSpeed, f32 accel) {
    f32 wishSpd = wishSpeed;
    if (wishSpd > SRC_AIR_WISH_CAP)
        wishSpd = SRC_AIR_WISH_CAP;

    f32 currentSpeed = m->vel[0] * wishDirX + m->vel[2] * wishDirZ;
    f32 addSpeed     = wishSpd - currentSpeed;
    if (addSpeed <= 0.0f)
        return;

    f32 accelSpeed = accel * wishSpeed * DT;
    if (accelSpeed > addSpeed)
        accelSpeed = addSpeed;

    m->vel[0] += accelSpeed * wishDirX;
    m->vel[2] += accelSpeed * wishDirZ;
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

/**
 * Ground movement: friction → accelerate → jump → collision step.
 *
 * Bhop: if A was buffered during airtime, jump immediately on landing WITHOUT
 * first applying friction, preserving the full horizontal speed from the air.
 */
#include <stdio.h>
#include <windows.h>
static void windows_show_message_box(const char *title, const char *message) {
#if defined(_WIN32) || defined(_WIN64)
    HWND consoleWindow = GetConsoleWindow();
    if (consoleWindow != NULL) {
        MessageBoxA(consoleWindow, message, title, MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(NULL, message, title, MB_OK | MB_ICONINFORMATION);
    }
#else
    fprintf(stderr, "%s: %s\n", title, message);
#endif
}
static void src_ground_step(struct MarioState *m) {
    f32 mag = m->intendedMag / 32.0f;
    f32 wishDirX = sins(m->intendedYaw);
    f32 wishDirZ = coss(m->intendedYaw);
    f32 wishSpeed = mag * SRC_MAX_SPEED;

    bool aHeld = (m->controller->buttonDown & A_BUTTON) != 0;
    bool justLanded = !sWasGrounded;
    bool isBhopFrame = justLanded && (sBhopBuffered || aHeld);

    if (isBhopFrame) {
        sBhopBuffered = FALSE;
        sBhopTimer    = 0;
    } else {
        src_apply_friction(m);
    }

    if (wishSpeed > SPEED_EPSILON) {
        src_accelerate(m, wishDirX, wishDirZ, wishSpeed, SRC_ACCEL);
    }

    f32 speedCap = isBhopFrame ? SRC_BHOP_SPEED_CAP : SRC_MAX_SPEED;
    src_clamp_horizontal_speed(m, speedCap);

    if (aHeld) {
        m->vel[1] = SRC_JUMP_SPEED;
        m->vel[1] -= SRC_EXTRA_GRAVITY;
        if (m->vel[1] < SRC_TERMINAL_VEL) m->vel[1] = SRC_TERMINAL_VEL;
        perform_air_step(m, 0);
        sWasGrounded = FALSE;
        //windows_show_message_box("Bhop!", "You performed a bunny hop!");
        return;
    }

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
static void src_air_step(struct MarioState *m) {
    if (m->controller->buttonDown & A_BUTTON) {
        sBhopBuffered = TRUE;
        sBhopTimer    = BHOP_WINDOW;
    } else if (sBhopTimer > 0) {
        sBhopTimer--;
        if (sBhopTimer == 0) {
            sBhopBuffered = FALSE;
        }
    }

    f32 mag      = m->intendedMag / 32.0f;
    f32 wishDirX = sins(m->intendedYaw);
    f32 wishDirZ = coss(m->intendedYaw);
    f32 wishSpeed = mag * SRC_MAX_SPEED;

    if (sOnSurf && m->floor != NULL) {
        f32 nx = m->floor->normal.x;
        f32 ny = m->floor->normal.y;
        f32 nz = m->floor->normal.z;

        src_clip_velocity_along_surface(m, nx, ny, nz);

        if (mag > SPEED_EPSILON) {
            src_air_accelerate(m, wishDirX, wishDirZ, wishSpeed, SRC_AIR_ACCEL);
        }

        m->vel[1] -= 4.0f;
        if (m->vel[1] < SRC_TERMINAL_VEL) m->vel[1] = SRC_TERMINAL_VEL;

        src_clip_velocity_along_surface(m, nx, ny, nz);

        f32 vx = m->vel[0];
        f32 vy = m->vel[1];
        f32 vz = m->vel[2];

        m->vel[1] += 4.0f;

        perform_air_step(m, 0);

        m->vel[0] = vx;
        m->vel[1] = vy;
        m->vel[2] = vz;

#define SURF_SNAP_DIST 40.0f

        if (m->floor != NULL
            && m->pos[1] <= m->floorHeight + SURF_SNAP_DIST
            && m->floor->normal.y >= SRC_SURF_MIN_NORMAL
            && m->floor->normal.y < SRC_WALKABLE_NORMAL) {
            m->pos[1] = m->floorHeight;
            src_clip_velocity_along_surface(m,
                m->floor->normal.x, m->floor->normal.y, m->floor->normal.z);
            sOnSurf      = TRUE;
            sWasGrounded = FALSE;
        } else if (m->floor != NULL
                && m->pos[1] <= m->floorHeight + SURF_SNAP_DIST
                && m->floor->normal.y >= SRC_WALKABLE_NORMAL) {
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
        src_air_accelerate(m, wishDirX, wishDirZ, wishSpeed, SRC_AIR_ACCEL);
    }

    src_clamp_horizontal_speed(m, SRC_BHOP_SPEED_CAP);

    m->vel[1] -= SRC_EXTRA_GRAVITY;
    if (m->vel[1] < SRC_TERMINAL_VEL) m->vel[1] = SRC_TERMINAL_VEL;

    s32 stepResult = perform_air_step(m, 0);

    if (stepResult == AIR_STEP_LANDED) {
        if (m->floor != NULL &&
            m->floor->normal.y >= SRC_SURF_MIN_NORMAL &&
            m->floor->normal.y <  SRC_WALKABLE_NORMAL) {
            src_clip_velocity_along_surface(m,
                m->floor->normal.x,
                m->floor->normal.y,
                m->floor->normal.z);
            sOnSurf      = TRUE;
            sWasGrounded = FALSE;
        } else {
            sOnSurf      = FALSE;
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
    m->pos[0] += sins(gFpsYaw) * cosP  * fwd    * SRC_NOCLIP_SPEED;
    m->pos[1] += sins(gFpsPitch)       * fwd    * SRC_NOCLIP_SPEED;
    m->pos[2] += coss(gFpsYaw) * cosP  * fwd    * SRC_NOCLIP_SPEED;

    m->pos[0] += -coss(gFpsYaw) * strafe * SRC_NOCLIP_SPEED;
    m->pos[2] +=  sins(gFpsYaw) * strafe * SRC_NOCLIP_SPEED;

    if (m->controller->buttonDown & A_BUTTON)  m->pos[1] += SRC_NOCLIP_SPEED;
    if (m->controller->buttonDown & Z_TRIG)    m->pos[1] -= SRC_NOCLIP_SPEED;

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
                 && (m->floor->normal.y >= SRC_WALKABLE_NORMAL);

    m->faceAngle[1] = gFpsYaw;

    if (onGround) {
        src_ground_step(m);
    } else {
        src_air_step(m);
    }
}
