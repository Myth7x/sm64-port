#include "manager.h"
#include "net.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <cstring>

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
extern "C" {
#include "types.h"
#include "game/level_update.h"
}

#define UDP_DEFAULT_HOST "127.0.0.1"
#define UDP_DEFAULT_PORT 7777
#define UDP_DEFAULT_ROOM "default"

static NetMarioSnapshot  g_snap;
static std::mutex        g_snap_mtx;
static std::atomic<bool> g_snap_valid{false};
static std::thread       g_net_thread;
static std::atomic<bool> g_net_running{false};

static void net_thread_func() {
    while (g_net_running.load(std::memory_order_relaxed)) {
        NetMarioSnapshot local;
        if (g_snap_valid.load(std::memory_order_acquire)) {
            {
                std::lock_guard<std::mutex> lk(g_snap_mtx);
                local = g_snap;
            }
            net_tick_snapshot(&local);
        } else {
            net_tick_snapshot(nullptr);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void udp_init(void) {
    net_init(UDP_DEFAULT_HOST, UDP_DEFAULT_PORT, UDP_DEFAULT_ROOM);
    g_net_running.store(true, std::memory_order_relaxed);
    g_net_thread = std::thread(net_thread_func);
}

void udp_snapshot(void) {
    if (!gMarioState) return;
    NetMarioSnapshot snap;
    net_snapshot_from_mario(gMarioState, &snap);
    {
        std::lock_guard<std::mutex> lk(g_snap_mtx);
        g_snap = snap;
    }
    g_snap_valid.store(true, std::memory_order_release);
}

void udp_shutdown(void) {
    g_net_running.store(false, std::memory_order_relaxed);
    if (g_net_thread.joinable()) g_net_thread.join();
    net_shutdown();
}

int udp_connected(void) {
    return net_is_connected();
}

const char *udp_status(void) {
    const char *msg = net_get_last_msg();
    if (msg && msg[0] != '\0') return msg;
    return "connecting...";
}

int udp_get_room_user_count(void) {
    const NetPlayer *players = net_get_players();
    int count = 0;
    for (int i = 0; i < NET_MAX_PLAYERS; i++) {
        if (players[i].active) count++;
    }
    return count;
}


