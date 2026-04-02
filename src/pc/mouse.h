#ifndef MOUSE_H
#define MOUSE_H

#include <stdbool.h>

extern int gMouseDeltaX;
extern int gMouseDeltaY;

void mouse_register_capture_callbacks(void (*do_capture)(void), void (*do_release)(void));
void mouse_accum(int dx, int dy);

void capture_mouse(void);
void release_mouse(void);
bool is_mouse_captured(void);

#endif
