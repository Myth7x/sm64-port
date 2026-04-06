#ifndef NET_H
#define NET_H

#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <unistd.h>
#endif

#include <stdint.h>

// ─── Protocol constants ───────────────────────────────────────────────────────

#define NET_VERSION       0x01
#define NET_MAX_PLAYERS   8
#define NET_HEADER_SIZE   6
#define NET_ROOM_NAME_LEN 16

// Opcodes
#define NET_OP_JOIN        0x01
#define NET_OP_LEAVE       0x02
#define NET_OP_PLAYER_POS  0x10
#define NET_OP_PLAYER_ANIM 0x11
#define NET_OP_GAME_EVENT  0x20
#define NET_OP_LEVEL_SYNC  0x30
#define NET_OP_PING        0x40
#define NET_OP_PONG        0x41
#define NET_OP_ERROR       0xFF

// Error reason codes
#define NET_ERR_ROOM_FULL        0x01
#define NET_ERR_VERSION_MISMATCH 0x02
#define NET_ERR_BAD_PASSWORD     0x03
#define NET_ERR_BANNED           0x04

// Game event types
#define NET_EVT_COIN_COLLECT 0x01
#define NET_EVT_STAR_GRAB    0x02
#define NET_EVT_DAMAGE       0x03
#define NET_EVT_DEATH        0x04
#define NET_EVT_INTERACT     0x05
#define NET_EVT_CUSTOM       0x06

// Player flags bitmask (FLAGS field in PLAYER_POS)
#define NET_FLAG_ON_GROUND     (1 << 0)
#define NET_FLAG_IN_WATER      (1 << 1)
#define NET_FLAG_CARRYING      (1 << 2)
#define NET_FLAG_INVINCIBLE    (1 << 3)
#define NET_FLAG_CAP_SHIFT     4        // bits 4-7: cap state

#define NET_INVALID_PID 0xFF
#define NET_BROADCAST   0xFF            // target = all in room

// ─── Packet sizes (header + payload) ─────────────────────────────────────────

#define NET_PKT_JOIN        24
#define NET_PKT_LEAVE        6
#define NET_PKT_PLAYER_POS  28
#define NET_PKT_PLAYER_ANIM 10
#define NET_PKT_GAME_EVENT  14
#define NET_PKT_LEVEL_SYNC  10
#define NET_PKT_PING        10
#define NET_PKT_ERROR        8

// ─── Remote player state ──────────────────────────────────────────────────────

typedef struct {
    uint8_t  active;
    uint8_t  pid;

    // Position & orientation
    float    x, y, z;
    int16_t  yaw, pitch, roll;
    uint16_t speed;
    uint32_t flags;

    // Animation
    uint16_t animId;
    uint8_t  animFrame;
    uint8_t  animInterp;

    // Level info
    uint8_t  level;
    uint8_t  area;
    uint8_t  act;
} NetPlayer;

// ─── Net state (singleton) ────────────────────────────────────────────────────

typedef struct {
#ifdef _WIN32
    SOCKET   sock;
#else
    int      sock;
#endif
    struct sockaddr_in server_addr;

    uint8_t  connected;
    uint8_t  pid;           // assigned by server
    uint8_t  sid;           // room/session id
    char     room[NET_ROOM_NAME_LEN];

    uint16_t seq_pos;       // sequence counter for PLAYER_POS
    uint16_t seq_anim;      // sequence counter for PLAYER_ANIM

    NetPlayer players[NET_MAX_PLAYERS];

    char     last_msg[128];
} NetState;

// ─── Thread-safe Mario state snapshot ────────────────────────────────────────

typedef struct {
    float    pos[3];
    int16_t  faceAngle[3];
    float    forwardVel;
    uint32_t action;
    uint32_t flags;
    int16_t  invincTimer;
    int      has_marioObj;
    int16_t  animID;
    int16_t  animFrame;
    int      heldObj;
} NetMarioSnapshot;

// ─── Public API ───────────────────────────────────────────────────────────────

int  net_init(const char *server_ip, uint16_t port, const char *room);

void net_snapshot_from_mario(struct MarioState *m, NetMarioSnapshot *out);
void net_tick_snapshot(const NetMarioSnapshot *snap);

void net_send_event(uint8_t type, uint8_t target, int32_t data);
void net_send_level_sync(uint8_t level, uint8_t area, uint8_t act);
void net_shutdown(void);

// Read-only access to remote player array (index = pid)
const NetPlayer *net_get_players(void);

int         net_is_connected(void);
const char *net_get_last_msg(void);
int        net_get_room_user_count(void);

#endif // NET_H