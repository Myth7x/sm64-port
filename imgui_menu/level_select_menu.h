#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void level_select_menu_init(void);
void level_select_menu_shutdown(void);
void level_select_menu_new_frame(void);
void level_select_menu_open(void);
void level_select_menu_close(void);
void level_select_menu_compose_frame(void);
void level_select_menu_render(void);

#ifdef __cplusplus
}
#endif
