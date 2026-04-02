#ifndef MOUSE_H
#define MOUSE_H

#include <stdbool.h>

extern int gMouseDeltaX;
extern int gMouseDeltaY;

void mouse_register_capture_callbacks(bool (*do_capture)(void), void (*do_release)(void));
void mouse_accum(int dx, int dy);

void capture_mouse(void);
void release_mouse(void);
bool is_mouse_captured(void);

unsigned long long pc_perf_counter(void);
unsigned long long pc_perf_freq(void);

#endif
