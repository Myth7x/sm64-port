#include "mouse.h"

int gMouseDeltaX = 0;
int gMouseDeltaY = 0;

static bool s_captured;
static void (*s_do_capture)(void);
static void (*s_do_release)(void);

void mouse_register_capture_callbacks(void (*do_capture)(void), void (*do_release)(void)) {
    s_do_capture = do_capture;
    s_do_release = do_release;
}

void mouse_accum(int dx, int dy) {
    gMouseDeltaX += dx;
    gMouseDeltaY += dy;
}

void capture_mouse(void) {
    if (s_do_capture) s_do_capture();
    s_captured = true;
}

void release_mouse(void) {
    s_captured = false;
    if (s_do_release) s_do_release();
}

bool is_mouse_captured(void) {
    return s_captured;
}