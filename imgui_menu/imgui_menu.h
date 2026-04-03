#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

void imgui_menu_init(void);
void imgui_menu_shutdown(void);
void imgui_menu_new_frame(void);
void imgui_menu_compose_frame(void);
void imgui_menu_render(void);
bool imgui_menu_wants_mouse_capture(void);

#ifdef __cplusplus
}
#endif
