#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void imgui_menu_init(void);
void imgui_menu_shutdown(void);
void imgui_menu_new_frame(void);
void imgui_menu_compose_frame(void);
void imgui_menu_render(void);

#ifdef __cplusplus
}
#endif
