#include "sm64_queue.h"

#if defined(TARGET_WINDOWS) || defined(TARGET_LINUX)

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <amqp.h>
#include <amqp_tcp_socket.h>

#include "sm64.h"
#include "game/level_update.h"
#include "game/area.h"

#ifdef TARGET_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
static void platform_sleep_ms(int ms) { Sleep((DWORD)ms); }
#else
#include <unistd.h>
static void platform_sleep_ms(int ms) { usleep((useconds_t)(ms * 1000)); }
#endif

#define AMQP_HOST     "46.224.24.39"
#define AMQP_PORT     5672
#define AMQP_VHOST    "/"
#define AMQP_USER     "guest"
#define AMQP_PASSWORD "guest"
#define AMQP_QUEUE    "sm64-mario-state"

typedef struct {
    float pos[3];
    float vel[3];
    int16_t faceAngle[3];
    uint32_t action;
    int16_t health;
    int16_t numCoins;
    int16_t numStars;
    int16_t level;
    int16_t area;
    int dirty;
} MarioSnapshot;

static pthread_t s_thread;
static pthread_mutex_t s_mutex;
static MarioSnapshot s_snapshot;
static volatile int s_running;

static int amqp_connect(amqp_connection_state_t conn) {
    amqp_socket_t *sock = amqp_tcp_socket_new(conn);
    if (!sock) return 0;
    if (amqp_socket_open(sock, AMQP_HOST, AMQP_PORT) != AMQP_STATUS_OK) return 0;

    amqp_rpc_reply_t r = amqp_login(conn, AMQP_VHOST, 0, 131072, 0,
        AMQP_SASL_METHOD_PLAIN, AMQP_USER, AMQP_PASSWORD);
    if (r.reply_type != AMQP_RESPONSE_NORMAL) return 0;

    amqp_channel_open(conn, 1);
    r = amqp_get_rpc_reply(conn);
    if (r.reply_type != AMQP_RESPONSE_NORMAL) return 0;

    amqp_queue_declare(conn, 1, amqp_cstring_bytes(AMQP_QUEUE),
        0, 0, 0, 1, amqp_empty_table);
    r = amqp_get_rpc_reply(conn);
    if (r.reply_type != AMQP_RESPONSE_NORMAL) return 0;

    return 1;
}

static void *amqp_worker(void *arg) {
    (void)arg;
    while (s_running) {
        amqp_connection_state_t conn = amqp_new_connection();
        if (!amqp_connect(conn)) {
            amqp_destroy_connection(conn);
            platform_sleep_ms(1000);
            continue;
        }

        while (s_running) {
            MarioSnapshot snap;
            int has_data = 0;

            pthread_mutex_lock(&s_mutex);
            if (s_snapshot.dirty) {
                snap = s_snapshot;
                s_snapshot.dirty = 0;
                has_data = 1;
            }
            pthread_mutex_unlock(&s_mutex);

            if (!has_data) {
                platform_sleep_ms(1);
                continue;
            }

            char json[512];
            snprintf(json, sizeof(json),
                "{\"pos\":[%.3f,%.3f,%.3f],"
                "\"vel\":[%.3f,%.3f,%.3f],"
                "\"faceAngle\":[%d,%d,%d],"
                "\"action\":%u,"
                "\"health\":%d,"
                "\"coins\":%d,"
                "\"stars\":%d,"
                "\"level\":%d,"
                "\"area\":%d}",
                snap.pos[0], snap.pos[1], snap.pos[2],
                snap.vel[0], snap.vel[1], snap.vel[2],
                (int)snap.faceAngle[0], (int)snap.faceAngle[1], (int)snap.faceAngle[2],
                (unsigned int)snap.action,
                (int)snap.health,
                (int)snap.numCoins,
                (int)snap.numStars,
                (int)snap.level,
                (int)snap.area
            );

            int status = amqp_basic_publish(conn, 1,
                amqp_cstring_bytes(""),
                amqp_cstring_bytes(AMQP_QUEUE),
                0, 0, NULL, amqp_cstring_bytes(json));
            if (status != AMQP_STATUS_OK) {
                break;
            }
        }

        amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
        amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(conn);
    }
    return NULL;
}

void sm64_queue_init(void) {
    pthread_mutex_init(&s_mutex, NULL);
    memset(&s_snapshot, 0, sizeof(s_snapshot));
    s_running = 1;
    pthread_create(&s_thread, NULL, amqp_worker, NULL);
}

void sm64_queue_shutdown(void) {
    s_running = 0;
    pthread_join(s_thread, NULL);
    pthread_mutex_destroy(&s_mutex);
}

void sm64_queue_send_mario_state(void) {
    struct MarioState *m = &gMarioStates[0];

    pthread_mutex_lock(&s_mutex);
    s_snapshot.pos[0] = m->pos[0];
    s_snapshot.pos[1] = m->pos[1];
    s_snapshot.pos[2] = m->pos[2];
    s_snapshot.vel[0] = m->vel[0];
    s_snapshot.vel[1] = m->vel[1];
    s_snapshot.vel[2] = m->vel[2];
    s_snapshot.faceAngle[0] = m->faceAngle[0];
    s_snapshot.faceAngle[1] = m->faceAngle[1];
    s_snapshot.faceAngle[2] = m->faceAngle[2];
    s_snapshot.action = m->action;
    s_snapshot.health = m->health;
    s_snapshot.numCoins = m->numCoins;
    s_snapshot.numStars = m->numStars;
    s_snapshot.level = gCurrLevelNum;
    s_snapshot.area = gCurrAreaIndex;
    s_snapshot.dirty = 1;
    pthread_mutex_unlock(&s_mutex);
}

#else

void sm64_queue_init(void) {}
void sm64_queue_shutdown(void) {}
void sm64_queue_send_mario_state(void) {}

#endif
