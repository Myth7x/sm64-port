#include "mouse.h"

int gMouseDeltaX = 0;
int gMouseDeltaY = 0;

void mouse_accum(int dx, int dy) {
    gMouseDeltaX += dx;
    gMouseDeltaY += dy;
}
