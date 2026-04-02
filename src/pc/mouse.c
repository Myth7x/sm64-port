#include "mouse.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <time.h>
#endif

int gMouseDeltaX = 0;
int gMouseDeltaY = 0;

static bool s_captured;
static bool (*s_do_capture)(void);
static void (*s_do_release)(void);

void mouse_register_capture_callbacks(bool (*do_capture)(void), void (*do_release)(void)) {
    s_do_capture = do_capture;
    s_do_release = do_release;
}

void mouse_accum(int dx, int dy) {
    gMouseDeltaX += dx;
    gMouseDeltaY += dy;
}

void capture_mouse(void) {
    if (s_do_capture) {
        if (s_do_capture())
            s_captured = true;
    } else {
        s_captured = true;
    }
}

void release_mouse(void) {
    s_captured = false;
    if (s_do_release) s_do_release();
}

bool is_mouse_captured(void) {
    return s_captured;
}

unsigned long long pc_perf_counter(void) {
#ifdef _WIN32
    LARGE_INTEGER v;
    QueryPerformanceCounter(&v);
    return (unsigned long long)v.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec * 1000000000ULL + (unsigned long long)ts.tv_nsec;
#endif
}

unsigned long long pc_perf_freq(void) {
#ifdef _WIN32
    LARGE_INTEGER v;
    QueryPerformanceFrequency(&v);
    return (unsigned long long)v.QuadPart;
#else
    return 1000000000ULL;
#endif
}