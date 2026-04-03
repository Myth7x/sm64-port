#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

void imgui_navbar_init(void);
void imgui_navbar_shutdown(void);
void imgui_navbar_compose_frame(void);
bool imgui_navbar_get_info_visible(void);

#ifdef __cplusplus
}
#endif
