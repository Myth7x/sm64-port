// net.cpp — SM64 UDP multiplayer client
// Drop into src/pc/ and add to the Makefile.
// Compiles cleanly with MinGW64 (MSYS2) and GCC/Clang on Linux.
//
// Link with -lws2_32 on Windows (already in sm64-port's Makefile for SDL).

#include "net.h"
#include "imgui_menu/toast_c.h"

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
extern "C" {
#include "types.h"
#include "sm64.h"
#include "game/mario.h"
}

#ifndef _WIN32
#include <fcntl.h>
#endif

#include <cstring>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#  pragma comment(lib, "ws2_32.lib")
#  define NET_CLOSE(s) closesocket(s)
#  define NET_INVALID  INVALID_SOCKET
   typedef int socklen_t;
#else
#  define NET_CLOSE(s) close(s)
#  define NET_INVALID  (-1)
#endif

// ─── Internal helpers ─────────────────────────────────────────────────────────

static NetState g_net;

// Write the 6-byte fixed header at buf[0..5]
static void write_header(uint8_t *buf, uint8_t op, uint8_t pid, uint8_t sid, uint16_t seq) {
    buf[0] = NET_VERSION;
    buf[1] = op;
    buf[2] = pid;
    buf[3] = sid;
    // Little-endian seq
    buf[4] = (uint8_t)(seq & 0xFF);
    buf[5] = (uint8_t)(seq >> 8);
}

// Copy 4-byte little-endian float into buf
static void write_f32(uint8_t *buf, float v) {
    memcpy(buf, &v, 4);
}

// Copy 2-byte little-endian int16 into buf
static void write_i16(uint8_t *buf, int16_t v) {
    buf[0] = (uint8_t)(v & 0xFF);
    buf[1] = (uint8_t)((uint16_t)v >> 8);
}

// Copy 2-byte little-endian uint16 into buf
static void write_u16(uint8_t *buf, uint16_t v) {
    buf[0] = (uint8_t)(v & 0xFF);
    buf[1] = (uint8_t)(v >> 8);
}

// Copy 4-byte little-endian uint32 into buf
static void write_u32(uint8_t *buf, uint32_t v) {
    buf[0] = (uint8_t)(v);
    buf[1] = (uint8_t)(v >> 8);
    buf[2] = (uint8_t)(v >> 16);
    buf[3] = (uint8_t)(v >> 24);
}

static void write_i32(uint8_t *buf, int32_t v) {
    write_u32(buf, (uint32_t)v);
}

static float    read_f32(const uint8_t *buf) { float v; memcpy(&v, buf, 4); return v; }
static int16_t  read_i16(const uint8_t *buf) { return (int16_t)((uint16_t)buf[0] | ((uint16_t)buf[1] << 8)); }
static uint16_t read_u16(const uint8_t *buf) { return (uint16_t)buf[0] | ((uint16_t)buf[1] << 8); }
static uint32_t read_u32(const uint8_t *buf) { return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24); }

// Sequence wrap-around comparison (window = 32768)
static int seq_is_newer(uint16_t incoming, uint16_t last) {
    uint16_t diff = (uint16_t)(incoming - last);
    return diff > 0 && diff < 32768u;
}

// ─── Packet senders ───────────────────────────────────────────────────────────

static void send_join(void) {
    uint8_t buf[NET_PKT_JOIN] = {0};
    write_header(buf, NET_OP_JOIN, NET_INVALID_PID, 0xFF, 0);
    strncpy((char *)buf + 6, g_net.room, NET_ROOM_NAME_LEN);
    buf[22] = NET_VERSION;
    buf[23] = 0x00;
    sendto(g_net.sock, (const char *)buf, NET_PKT_JOIN, 0,
           (struct sockaddr *)&g_net.server_addr, sizeof(g_net.server_addr));
}

static void send_player_pos_snap(const NetMarioSnapshot *s) {
    uint8_t buf[NET_PKT_PLAYER_POS] = {0};
    uint32_t flags = 0;
    if (!(s->action & (ACT_FLAG_AIR | ACT_FLAG_SWIMMING))) flags |= NET_FLAG_ON_GROUND;
    if (s->action & ACT_FLAG_SWIMMING)                     flags |= NET_FLAG_IN_WATER;
    if (s->heldObj)                                        flags |= NET_FLAG_CARRYING;
    if (s->invincTimer > 0)                                flags |= NET_FLAG_INVINCIBLE;
    if (s->flags & MARIO_WING_CAP)        flags |= (1 << NET_FLAG_CAP_SHIFT);
    else if (s->flags & MARIO_METAL_CAP)  flags |= (2 << NET_FLAG_CAP_SHIFT);
    else if (s->flags & MARIO_VANISH_CAP) flags |= (3 << NET_FLAG_CAP_SHIFT);
    write_header(buf, NET_OP_PLAYER_POS, g_net.pid, g_net.sid, g_net.seq_pos++);
    write_f32(buf + 6,  s->pos[0]);
    write_f32(buf + 10, s->pos[1]);
    write_f32(buf + 14, s->pos[2]);
    write_i16(buf + 18, s->faceAngle[1]);
    write_i16(buf + 20, s->faceAngle[0]);
    write_i16(buf + 22, s->faceAngle[2]);
    write_u32(buf + 24, flags);
    sendto(g_net.sock, (const char *)buf, NET_PKT_PLAYER_POS, 0,
           (struct sockaddr *)&g_net.server_addr, sizeof(g_net.server_addr));
}

static void send_player_anim_snap(const NetMarioSnapshot *s) {
    uint8_t buf[NET_PKT_PLAYER_ANIM];
    write_header(buf, NET_OP_PLAYER_ANIM, g_net.pid, g_net.sid, g_net.seq_anim++);
    write_u16(buf + 6, (uint16_t)s->animID);
    buf[8] = (uint8_t)s->animFrame;
    buf[9] = 0;
    sendto(g_net.sock, (const char *)buf, NET_PKT_PLAYER_ANIM, 0,
           (struct sockaddr *)&g_net.server_addr, sizeof(g_net.server_addr));
}

static void handle_player_pos(const uint8_t *buf, int len) {
    if (len < NET_PKT_PLAYER_POS) return;

    uint8_t  pid = buf[2];
    if (pid >= NET_MAX_PLAYERS || pid == g_net.pid) return;

    NetPlayer *p = &g_net.players[pid];
    int was_active = p->active;
    p->active = 1;
    p->pid    = pid;
    p->x      = read_f32(buf + 6);
    p->y      = read_f32(buf + 10);
    p->z      = read_f32(buf + 14);
    p->yaw    = read_i16(buf + 18);
    p->pitch  = read_i16(buf + 20);
    p->roll   = read_i16(buf + 22);
    p->speed  = read_u16(buf + 24);
    p->flags  = read_u32(buf + 24);
    if (!was_active) {
        char body[32];
        snprintf(body, sizeof(body), "PID %d", pid);
        toast_push("Player joined", body);
    }
}

static void handle_player_anim(const uint8_t *buf, int len) {
    if (len < NET_PKT_PLAYER_ANIM) return;

    uint8_t pid = buf[2];
    if (pid >= NET_MAX_PLAYERS || pid == g_net.pid) return;

    NetPlayer *p = &g_net.players[pid];
    p->animId    = read_u16(buf + 6);
    p->animFrame = buf[8];
    p->animInterp= buf[9];
}

static void handle_game_event(const uint8_t *buf, int len) {
    if (len < NET_PKT_GAME_EVENT) return;

    uint8_t sender = buf[2];
    uint8_t type   = buf[6];
    uint8_t target = buf[7];

    // If sender disconnected (DEATH with data=0xFF), deactivate their slot
    if (type == NET_EVT_DEATH && target == NET_BROADCAST) {
        if (sender < NET_MAX_PLAYERS && g_net.players[sender].active) {
            char body[32];
            snprintf(body, sizeof(body), "PID %d", sender);
            toast_push("Player left", body);
        }
        if (sender < NET_MAX_PLAYERS)
            g_net.players[sender].active = 0;
    }

    // Extend here: trigger coin FX, star collection, etc.
    (void)target;
}

static void handle_level_sync(const uint8_t *buf, int len) {
    if (len < NET_PKT_LEVEL_SYNC) return;

    uint8_t pid = buf[2];
    if (pid >= NET_MAX_PLAYERS || pid == g_net.pid) return;

    NetPlayer *p = &g_net.players[pid];
    p->level = buf[6];
    p->area  = buf[7];
    p->act   = buf[8];
}

static void handle_join_ack(const uint8_t *buf, int len) {
    if (len < NET_PKT_JOIN) return;
    g_net.pid = buf[2];
    g_net.sid = buf[3];
    g_net.connected = 1;
    snprintf(g_net.last_msg, sizeof(g_net.last_msg), "PID:%d sid:%d", g_net.pid, g_net.sid);
    toast_push("Connected", g_net.last_msg);
    printf("[net] joined room \"%s\" as pid=%d sid=%d\n", g_net.room, g_net.pid, g_net.sid);
}

static void handle_error(const uint8_t *buf, int len) {
    if (len < NET_PKT_ERROR) return;
    const char *reasons[] = { "?", "room full", "version mismatch", "bad password", "banned" };
    uint8_t reason = buf[6];
    const char *msg = reason < 5 ? reasons[reason] : "unknown";
    snprintf(g_net.last_msg, sizeof(g_net.last_msg), "error: %s", msg);
    toast_push("Network Error", msg);
    printf("[net] server error: %s\n", msg);
}

// Drain the socket receive buffer (non-blocking)
static void recv_packets(void) {
    uint8_t buf[256];
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);

    for (int i = 0; i < 64; i++) {  // process at most 64 packets per tick
#ifdef _WIN32
        int n = recvfrom(g_net.sock, (char *)buf, sizeof(buf), 0,
                         (struct sockaddr *)&from, &fromlen);
        if (n == SOCKET_ERROR) break;
#else
        ssize_t n = recvfrom(g_net.sock, buf, sizeof(buf), 0,
                             (struct sockaddr *)&from, &fromlen);
        if (n <= 0) break;
#endif
        if (n < NET_HEADER_SIZE) continue;
        if (buf[0] != NET_VERSION) continue;

        uint8_t op = buf[1];
        switch (op) {
            case NET_OP_JOIN:        handle_join_ack(buf, (int)n);    break;
            case NET_OP_PLAYER_POS:  handle_player_pos(buf, (int)n);  break;
            case NET_OP_PLAYER_ANIM: handle_player_anim(buf, (int)n); break;
            case NET_OP_GAME_EVENT:  handle_game_event(buf, (int)n);  break;
            case NET_OP_LEVEL_SYNC:  handle_level_sync(buf, (int)n);  break;
            case NET_OP_ERROR:       handle_error(buf, (int)n);        break;
            default: break;
        }
    }
}

// ─── Public API ───────────────────────────────────────────────────────────────

int net_init(const char *server_ip, uint16_t port, const char *room) {
    memset(&g_net, 0, sizeof(g_net));
    g_net.pid = NET_INVALID_PID;
    strncpy(g_net.room, room, NET_ROOM_NAME_LEN - 1);

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "[net] WSAStartup failed\n");
        return -1;
    }
    g_net.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (g_net.sock == INVALID_SOCKET) {
        fprintf(stderr, "[net] socket() failed: %d\n", WSAGetLastError());
        return -1;
    }
    // Non-blocking
    u_long nb = 1;
    ioctlsocket(g_net.sock, FIONBIO, &nb);
#else
    g_net.sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_net.sock < 0) {
        perror("[net] socket");
        return -1;
    }
    // Non-blocking
    int flags = fcntl(g_net.sock, F_GETFL, 0);
    fcntl(g_net.sock, F_SETFL, flags | O_NONBLOCK);
#endif

    memset(&g_net.server_addr, 0, sizeof(g_net.server_addr));
    g_net.server_addr.sin_family      = AF_INET;
    g_net.server_addr.sin_port        = htons(port);
    g_net.server_addr.sin_addr.s_addr = inet_addr(server_ip);

    printf("[net] connecting to %s:%d room=\"%s\"\n", server_ip, port, room);
    send_join();
    return 0;
}

void net_snapshot_from_mario(struct MarioState *m, NetMarioSnapshot *out) {
    if (!m) { memset(out, 0, sizeof(*out)); return; }
    out->pos[0]       = m->pos[0];
    out->pos[1]       = m->pos[1];
    out->pos[2]       = m->pos[2];
    out->faceAngle[0] = m->faceAngle[0];
    out->faceAngle[1] = m->faceAngle[1];
    out->faceAngle[2] = m->faceAngle[2];
    out->forwardVel   = m->forwardVel;
    out->action       = m->action;
    out->flags        = m->flags;
    out->invincTimer  = m->invincTimer;
    out->heldObj      = (m->heldObj != NULL);
    if (m->marioObj) {
        out->has_marioObj = 1;
        out->animID    = (int16_t)m->marioObj->header.gfx.animInfo.animID;
        out->animFrame = (int16_t)(m->marioObj->header.gfx.animInfo.animFrame & 0xFF);
    } else {
        out->has_marioObj = 0;
        out->animID    = 0;
        out->animFrame = 0;
    }
}

void net_tick_snapshot(const NetMarioSnapshot *snap) {
    recv_packets();
    if (!g_net.connected) {
        static int retry = 0;
        if (++retry >= 60) { retry = 0; send_join(); }
        return;
    }
    if (!snap || !snap->has_marioObj) return;
    send_player_pos_snap(snap);
    static int anim_tick = 0;
    if (++anim_tick >= 2) {
        anim_tick = 0;
        send_player_anim_snap(snap);
    }
}

void net_send_event(uint8_t type, uint8_t target, int32_t data) {
    if (!g_net.connected) return;
    uint8_t buf[NET_PKT_GAME_EVENT];
    write_header(buf, NET_OP_GAME_EVENT, g_net.pid, g_net.sid, 0);
    buf[6] = type;
    buf[7] = target;
    write_i32(buf + 8, data);
    buf[12] = 0;
    buf[13] = 0;
    sendto(g_net.sock, (const char *)buf, NET_PKT_GAME_EVENT, 0,
           (struct sockaddr *)&g_net.server_addr, sizeof(g_net.server_addr));
}

void net_send_level_sync(uint8_t level, uint8_t area, uint8_t act) {
    if (!g_net.connected) return;
    uint8_t buf[NET_PKT_LEVEL_SYNC];
    write_header(buf, NET_OP_LEVEL_SYNC, g_net.pid, g_net.sid, 0);
    buf[6] = level;
    buf[7] = area;
    buf[8] = act;
    buf[9] = 0;
    sendto(g_net.sock, (const char *)buf, NET_PKT_LEVEL_SYNC, 0,
           (struct sockaddr *)&g_net.server_addr, sizeof(g_net.server_addr));
}

void net_shutdown(void) {
    if (!g_net.connected) return;
    uint8_t buf[NET_PKT_LEAVE];
    write_header(buf, NET_OP_LEAVE, g_net.pid, g_net.sid, 0);
    sendto(g_net.sock, (const char *)buf, NET_PKT_LEAVE, 0,
           (struct sockaddr *)&g_net.server_addr, sizeof(g_net.server_addr));
    NET_CLOSE(g_net.sock);
#ifdef _WIN32
    WSACleanup();
#endif
    memset(&g_net, 0, sizeof(g_net));
    printf("[net] disconnected\n");
}

const NetPlayer *net_get_players(void) {
    return g_net.players;
}

int net_is_connected(void) {
    return g_net.connected;
}

const char *net_get_last_msg(void) {
    return g_net.last_msg;
}

int net_get_room_user_count(void) {
    const NetPlayer *players = net_get_players();
    int count = 0;
    for (int i = 0; i < NET_MAX_PLAYERS; i++) {
        if (players[i].active) count++;
    }
    return count;
}